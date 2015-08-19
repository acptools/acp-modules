package net.acprog.modules.messenger;

import java.util.LinkedList;
import java.util.Queue;

//Requires JSSC: https://github.com/scream3r/java-simple-serial-connector/releases
import jssc.SerialPort;
import jssc.SerialPortException;

/**
 * Thread-safe implementation of messenger implementing the GEP.
 */
public class GEPMessenger {

	/**
	 * Listener for receiving messages sent over the channel.
	 */
	interface MessageListener {
		/**
		 * Invoked when a message is received.
		 * 
		 * @param tag
		 *            the tag of message or a negative number, if the received
		 *            message does not contain a tag
		 * @param message
		 *            the message
		 */
		void onMessageReceived(int tag, byte[] message);
	}

	/**
	 * Encapsulation of request to send a message.
	 */
	public static final class SendRequest {
		/**
		 * Submitted content of message.
		 */
		private final byte[] message;

		/**
		 * Tag associated to sent message.
		 */
		private final int tag;

		/**
		 * Indicates whether message has been successfully sent.
		 */
		private boolean successed;

		/**
		 * Indicates that the message processing was completed.
		 */
		private boolean completed = false;

		/**
		 * Lock that manages waiting for completion.
		 */
		private Object waitLock = new Object();

		/**
		 * Constructs a new request to send a message.
		 * 
		 * @param tag
		 *            the tag associated to the message.
		 * @param message
		 *            the binary message.
		 */
		private SendRequest(int tag, byte[] message) {
			this.tag = tag;
			if (message != null) {
				this.message = message.clone();
			} else {
				this.message = null;
			}
		}

		/**
		 * Marks that the request was completed.
		 * 
		 * @param success
		 *            the indicator whether the request was successfully
		 *            completed.
		 */
		private void markCompleted(boolean success) {
			synchronized (waitLock) {
				if (completed) {
					return;
				}

				completed = true;
				successed = success;
				waitLock.notifyAll();
			}
		}

		/**
		 * Waits for message completion by messenger.
		 */
		public void waitForCompletion() throws InterruptedException {
			synchronized (waitLock) {
				if (!completed) {
					waitLock.wait();
				}
			}
		}

		/**
		 * Waits for message completion by messenger or timeout.
		 */
		public void waitForCompletion(long timeout) throws InterruptedException {
			synchronized (waitLock) {
				if (!completed) {
					waitLock.wait(timeout);
				}
			}
		}

		/**
		 * Returns whether processing of the message by messenger is completed.
		 * 
		 * @return true, if the processing of the message is completed, false
		 *         otherwise.
		 */
		public boolean isCompleted() {
			synchronized (waitLock) {
				return completed;
			}
		}

		/**
		 * Returns whether the message was sent successfully.
		 * 
		 * @return true, if the message was sent successfully, false otherwise.
		 */
		public boolean isSent() {
			synchronized (waitLock) {
				return completed && successed;
			}
		}
	}

	/**
	 * Byte indicating start of a new message
	 */
	private final int MESSAGE_START_BYTE = 0x0C;

	/**
	 * Byte indicating end of the message without tag
	 */
	private final int MESSAGE_END_BYTE = 0x03;

	/**
	 * Byte indicating end of the message with tag
	 */
	private final int MESSAGE_END_WITH_TAG_BYTE = 0x06;

	/**
	 * States of implementation during receiving bytes according to the
	 * protocol.
	 */
	private enum ProtocolState {
		/**
		 * Waits for the beginning of a new message
		 */
		WAIT_START,
		/**
		 * Waits for a high nibble of the next message byte
		 */
		WAIT_HIGH_NIBBLE,
		/**
		 * Waits for a low nibble of the next message byte
		 */
		WAIT_LOW_NIBBLE,
		/**
		 * Waits for a CRC byte after receiving marker indicating end of a
		 * message without a tag
		 */
		WAIT_CRC,
		/**
		 * Waits for a CRC byte after receiving marker indicating end of a
		 * message with a tag
		 */
		WAIT_CRC_WITH_TAG
	}

	/**
	 * Listener for receiving messages.
	 */
	private final MessageListener messageListener;

	/**
	 * Identification of serial port.
	 */
	private final String portName;

	/**
	 * Baud rate of serial connection
	 */
	private final int baudRate;

	/**
	 * Maximal length of a message.
	 */
	private final int maxMessageLength;

	/**
	 * Queue with messages to send.
	 */
	private final Queue<SendRequest> messagesToSend = new LinkedList<GEPMessenger.SendRequest>();

	/**
	 * Indicates that messenger thread should be terminated as soon as possible.
	 */
	private volatile boolean stopFlag = false;

	/**
	 * Precomputed table for CRC checksums.
	 */
	private final short[] crcTable = new short[256];

	/**
	 * Thread for receiving and sending messages. Null, if the messenger is not
	 * running.
	 */
	private Thread communicationThread;

	/**
	 * Constructs a messenger.
	 * 
	 * @param portName
	 *            the identification of serial port
	 * @param messageListener
	 *            the message listener.
	 */
	public GEPMessenger(String portName, int baudRate, int maxMessageLength, MessageListener messageListener) {
		this.portName = portName;
		this.baudRate = Math.abs(baudRate);
		this.maxMessageLength = Math.abs(maxMessageLength);
		this.messageListener = messageListener;

		initializeCRCTable();
	}

	/**
	 * Returns whether messenger is running.
	 * 
	 * @return true, if the messenger is running, false otherwise.
	 */
	public synchronized boolean isRunning() {
		return (communicationThread != null);
	}

	/**
	 * Starts the messenger, if the messenger is not running.
	 */
	public synchronized void start() {
		if (isRunning()) {
			throw new RuntimeException("Messenger is already running.");
		}

		// Create communication thread
		communicationThread = new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					communicate();
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					synchronized (GEPMessenger.this) {
						// Store that there is no running communication thread
						communicationThread = null;
						// Clear queue with messages to send
						synchronized (messagesToSend) {
							while (!messagesToSend.isEmpty()) {
								SendRequest sr = messagesToSend.poll();
								sr.markCompleted(false);
							}
						}
					}
				}
			}
		});

		// Start the communication thread
		stopFlag = false;
		communicationThread.setDaemon(true);
		communicationThread.setName("GEPMessenger - Communication thread");
		communicationThread.start();
	}

	/**
	 * Stops the messenger.
	 * 
	 * @param blocked
	 *            true, if the method should be blocked until the communication
	 *            terminates, false otherwise.
	 */
	public void stop(boolean blocked) {
		Thread stoppingThread = null;

		synchronized (this) {
			if (communicationThread == null) {
				return;
			}

			stoppingThread = communicationThread;
			stopFlag = true;
		}

		if (stoppingThread != null) {
			try {
				stoppingThread.interrupt();
				if (blocked) {
					stoppingThread.join();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	/**
	 * Sends a message and returns a send request objects that provides status
	 * information.
	 * 
	 * @param message
	 *            the binary message.
	 * @param tag
	 *            the tag to be associated with the message.
	 */
	public synchronized SendRequest sendMessage(byte[] message, int tag) {
		if (tag >= 256 * 256) {
			throw new RuntimeException("Message tag cannot be greater than 65535.");
		}

		// Create request object
		SendRequest request = new SendRequest(tag, message);

		// Add request object to the queue with messages to send
		synchronized (messagesToSend) {
			messagesToSend.offer(request);
		}

		return request;
	}

	/**
	 * Sends a message and returns a send request objects that provides status
	 * information.
	 * 
	 * @param message
	 *            the binary message.
	 */
	public synchronized SendRequest sendMessage(byte[] message) {
		return sendMessage(message, -1);
	}

	/**
	 * Core method implementing communication using the protocol.
	 */
	private void communicate() {
		// Compute sleep interval with respect to baud rate.
		long nanosPerByte = Math.max((1000000000L / baudRate) * 8, 100);
		final int nanosSleep = (int) (nanosPerByte % 1000000);
		final long millisSleep = nanosPerByte / 1000000;

		// Specify maximal buffer size (message length + 2 bytes for tag)
		final int maxBufferSize = maxMessageLength + 2;

		try {
			// Create serial port
			SerialPort serialPort = new SerialPort(portName);

			// Open port
			serialPort.openPort();
			serialPort.setParams(baudRate, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

			// We start waiting for a message start
			ProtocolState state = ProtocolState.WAIT_START;

			short[] messageBuffer = new short[maxBufferSize];
			int receivedMessageBytes = 0;
			short crc = 0;

			while (!stopFlag) {
				// Read received data
				int[] receivedData = serialPort.readIntArray();
				if ((receivedData != null) && (receivedData.length != 0)) {
					// Process received bytes
					for (final int receivedByte : receivedData) {
						// After receiving the message start byte in a state
						// other than waiting for CRC, we restart receiving of
						// the message
						if ((receivedByte == MESSAGE_START_BYTE) && (state != ProtocolState.WAIT_CRC)
								&& (state != ProtocolState.WAIT_CRC_WITH_TAG)) {
							state = ProtocolState.WAIT_START;
						}

						switch (state) {
						case WAIT_START:
							if (receivedByte == MESSAGE_START_BYTE) {
								state = ProtocolState.WAIT_HIGH_NIBBLE;
								crc = 0;
								receivedMessageBytes = 0;
							}
							break;

						case WAIT_HIGH_NIBBLE:
							if (receivedByte == MESSAGE_END_BYTE) {
								state = ProtocolState.WAIT_CRC;
							} else if (receivedByte == MESSAGE_END_WITH_TAG_BYTE) {
								state = ProtocolState.WAIT_CRC_WITH_TAG;
							} else if (receivedMessageBytes >= maxBufferSize) {
								state = ProtocolState.WAIT_START;
							} else {
								final int nibble = receivedByte / 16;
								if (nibble == ((receivedByte ^ 0x0F) & 0x0F)) {
									messageBuffer[receivedMessageBytes] = (short) (nibble * 16);
									receivedMessageBytes++;
									state = ProtocolState.WAIT_LOW_NIBBLE;
								} else {
									state = ProtocolState.WAIT_START;
								}
							}
							break;

						case WAIT_LOW_NIBBLE:
							final int nibble = receivedByte / 16;
							if (nibble == ((receivedByte ^ 0x0F) & 0x0F)) {
								messageBuffer[receivedMessageBytes - 1] += nibble;
								crc = updateCRC(messageBuffer[receivedMessageBytes - 1], crc);
								state = ProtocolState.WAIT_HIGH_NIBBLE;
							} else {
								state = ProtocolState.WAIT_START;
							}
							break;

						case WAIT_CRC:
							if (receivedByte == crc) {
								handleReceivedMessage(messageBuffer, receivedMessageBytes, -1);
							}
							state = ProtocolState.WAIT_START;
							break;

						case WAIT_CRC_WITH_TAG:
							if ((receivedByte == crc) && (receivedMessageBytes >= 2)) {
								final int messageTag = messageBuffer[receivedMessageBytes - 2] * 256
										+ messageBuffer[receivedMessageBytes - 1];
								receivedMessageBytes -= 2;
								handleReceivedMessage(messageBuffer, receivedMessageBytes, messageTag);
							}
							state = ProtocolState.WAIT_START;
							break;
						}
					}
				} else {
					try {
						// Sleep time required to receive one byte
						Thread.sleep(millisSleep, nanosSleep);
					} catch (InterruptedException ignore) {
						// Nothing to do
					}
				}

				// Check stop flag
				if (stopFlag) {
					break;
				}

				// If there are messages to send, we send the oldest one.
				SendRequest sendRequest = null;
				synchronized (messagesToSend) {
					sendRequest = messagesToSend.poll();
				}

				if (sendRequest != null) {
					handleSendRequest(serialPort, sendRequest);
				}
			}

			// Close port
			serialPort.closePort();
		} catch (SerialPortException e) {
			System.err.println("Communication over serial port " + portName + " at " + baudRate + "failed.");
			e.printStackTrace();
		}
	}

	/**
	 * Handles a received message.
	 * 
	 * @param messageBuffer
	 *            the buffer that stores the received message.
	 * @param messageLength
	 *            the length of the received message.
	 * @param tag
	 *            the tag associated with the message.
	 */
	private void handleReceivedMessage(short[] messageBuffer, int messageLength, int tag) {
		byte[] message = new byte[messageLength];
		for (int i = 0; i < messageLength; i++) {
			message[i] = (byte) messageBuffer[i];
		}

		if (messageListener != null) {
			messageListener.onMessageReceived(tag, message);
		}
	}

	/**
	 * Handles a request to send a message.
	 * 
	 * @param serial
	 *            the serial port to be used to send the message.
	 * @param sendRequest
	 *            the request to send a message.
	 */
	private void handleSendRequest(SerialPort serial, SendRequest sendRequest) {
		boolean allOK = true;
		try {
			short crc = 0;
			// Write byte starting a message
			allOK = allOK && serial.writeInt(MESSAGE_START_BYTE);

			// Compute length of data to be sent (including encoded tag)
			int rawMessageLength = (sendRequest.message == null) ? 0 : sendRequest.message.length;
			if (sendRequest.tag >= 0) {
				rawMessageLength += 2;
			}

			// Create raw message content
			final int[] rawMessage = new int[rawMessageLength];
			int writeIdx = 0;
			if (sendRequest.message != null) {
				for (final byte b : sendRequest.message) {
					rawMessage[writeIdx] = b & 0xff;
					writeIdx++;
				}
			}

			if (sendRequest.tag >= 0) {
				rawMessage[writeIdx] = sendRequest.tag / 256;
				writeIdx++;
				rawMessage[writeIdx] = sendRequest.tag % 256;
			}

			// Write message content with each byte encoded as two nibbles
			final int[] twoNibbles = new int[2];
			for (final int b : rawMessage) {
				crc = updateCRC((short) b, crc);
				twoNibbles[0] = b / 16;
				twoNibbles[1] = b % 16;
				twoNibbles[0] = twoNibbles[0] * 16 + ((twoNibbles[0] ^ 0x0F) & 0x0F);
				twoNibbles[1] = twoNibbles[1] * 16 + ((twoNibbles[1] ^ 0x0F) & 0x0F);

				allOK = allOK && serial.writeIntArray(twoNibbles);
				if (!allOK) {
					break;
				}
			}

			if (sendRequest.tag >= 0) {
				allOK = allOK && serial.writeInt(MESSAGE_END_WITH_TAG_BYTE);
			} else {
				allOK = allOK && serial.writeInt(MESSAGE_END_BYTE);
			}

			allOK = allOK && serial.writeInt(crc);
		} catch (SerialPortException e) {
			e.printStackTrace();
			allOK = false;
		}

		sendRequest.markCompleted(allOK);
	}

	/**
	 * Updates CRC8 checksum after adding a new byte of data.
	 * 
	 * @param data
	 *            the new byte of data
	 * @param crc
	 *            the current CRC8 checksum
	 * @return updates CRC8 checksum
	 */
	private short updateCRC(short aByte, short crc) {
		return (short) (crcTable[(aByte ^ crc) & 0xFF]);
	}

	/**
	 * Initializes table for fast computation of CRC8 checksums
	 */
	private void initializeCRCTable() {
		// based on:
		// http://stackoverflow.com/questions/25284556/translate-crc8-from-c-to-java
		final int polynomial = 0x8C;
		for (int dividend = 0; dividend < 256; dividend++) {
			int remainder = dividend;// << 8;
			for (int bit = 0; bit < 8; ++bit)
				if ((remainder & 0x01) != 0)
					remainder = (remainder >>> 1) ^ polynomial;
				else
					remainder >>>= 1;
			crcTable[dividend] = (short) remainder;
		}
	}
}
