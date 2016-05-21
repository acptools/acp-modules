package net.acprog.modules.messenger;

import net.acprog.modules.messenger.GEPMessenger.MessageListener;
import net.acprog.modules.messenger.GEPMessenger.SendRequest;

/**
 * ' Connector to a remote registry based on GEP message protocol.
 */
public class GEPRegistryConnector implements RegistryConnector {

	/**
	 * Code of request for reading value of a register.
	 */
	private final static int READ_REGISTRY_REQUEST = 0x01;

	/**
	 * Code of Request for writing value of a register.
	 */
	private final static int WRITE_REGISTRY_REQUEST = 0x02;

	/**
	 * Code of response indicating an unknown request or failed request.
	 */
	@SuppressWarnings("unused")
	private final static int REQUEST_FAILED_RESPONSE = 0x00;

	/**
	 * Code of response indicating that request was completed.
	 */
	private final static int REQUEST_OK_RESPONSE = 0x01;

	/**
	 * Code of response indicating that write request failed due to unwritable
	 * register.
	 */
	@SuppressWarnings("unused")
	private final static int UNWRITABLE_REGISTER_RESPONSE = 0x02;

	/**
	 * Messenger that allows communication with a remote registry using GEP
	 * protocol.
	 */
	private final GEPMessenger messenger;

	/**
	 * Maximal time in milliseconds that is wait for completion of a request.
	 */
	private long operationTimeout = 2000;

	/**
	 * Counter to generate "unique" request tags.
	 */
	private int tagCounter = 300;

	/**
	 * Tag associated to pending request.
	 */
	private int pendingRequestTag = -1;

	/**
	 * Response to request.
	 */
	private byte[] receivedResponse = null;

	/**
	 * Internal lock that manages processing received messages.
	 */
	private final Object requestLock = new Object();

	/**
	 * Constructs new registry connecter based on GEP messenger.
	 * 
	 * @param portName
	 *            the name of serial port.
	 * @param baudRate
	 *            the baud rate.
	 */
	public GEPRegistryConnector(String portName, int baudRate) {
		messenger = new GEPMessenger(portName, baudRate, 30, new MessageListener() {

			@Override
			public void onMessageReceived(int tag, byte[] message) {
				handleMessage(tag, message);
			}
		});
	}

	public synchronized void start() {
		messenger.start();
	}

	public synchronized void stop() {
		messenger.stop(true);
	}

	@Override
	public synchronized int readRegister(int registerId) throws RuntimeException {
		if ((registerId < 0) || (registerId >= 128 * 256)) {
			throw new RuntimeException("ID (" + registerId + ") of register is out of range.");
		}

		// Prepare message with request
		byte[] request = null;
		if (registerId < 128) {
			request = new byte[2];
			request[1] = (byte) registerId;
		} else {
			request = new byte[3];
			request[1] = (byte) ((registerId / 256) | 0x80);
			request[2] = (byte) (registerId % 256);
		}
		request[0] = READ_REGISTRY_REQUEST;

		// Send request and process response
		try {
			byte[] response = sendRequest(request);

			if (response == null) {
				throw new RuntimeException("No response from registry.");
			}

			if (response[0] != REQUEST_OK_RESPONSE) {
				throw new RuntimeException("Request failed on registry.");
			}

			return decodeNumber(response, 1);
		} catch (Exception e) {
			throw new RuntimeException("Read operation failed.", e);
		}
	}

	@Override
	public synchronized void writeRegister(int registerId, int value) throws RuntimeException {
		if ((registerId < 0) || (registerId >= 128 * 256)) {
			throw new RuntimeException("ID (" + registerId + ") of register is out of range.");
		}

		// Prepare message with request
		byte[] request;
		byte[] encodedValue = encodeNumber(value);
		if (registerId < 128) {
			request = new byte[2 + encodedValue.length];
			request[1] = (byte) registerId;
			System.arraycopy(encodedValue, 0, request, 2, encodedValue.length);
		} else {
			request = new byte[3 + encodedValue.length];
			request[1] = (byte) ((registerId / 256) | 0x80);
			request[2] = (byte) (registerId % 256);
			System.arraycopy(encodedValue, 0, request, 3, encodedValue.length);
		}
		request[0] = WRITE_REGISTRY_REQUEST;

		// Send request and process response
		try {
			byte[] response = sendRequest(request);

			if (response == null) {
				throw new RuntimeException("No response from registry.");
			}

			if (response[0] != REQUEST_OK_RESPONSE) {
				throw new RuntimeException("Request failed on registry.");
			}
		} catch (Exception e) {
			throw new RuntimeException("Write operation failed.", e);
		}
	}

	/**
	 * Sends a request and waits for response.
	 * 
	 * @param request
	 *            the encoded request.
	 * @return the encoded response or null, if no response was received.
	 */
	private byte[] sendRequest(byte[] request) throws RuntimeException {
		long operationStart = System.currentTimeMillis();
		try {
			// Initialize
			int messageTag;
			synchronized (requestLock) {
				tagCounter = (tagCounter + 1) % 1000;
				pendingRequestTag = tagCounter;
				messageTag = pendingRequestTag;
				receivedResponse = null;
			}

			// Send message with request
			SendRequest sr = messenger.sendMessage(request, messageTag);

			// Wait for sending of the request.
			try {
				if (operationTimeout > 0) {
					sr.waitForCompletion(operationTimeout);
				} else {
					sr.waitForCompletion();
				}
			} catch (InterruptedException e) {
				throw new RuntimeException("Request interrupted.", e);
			}

			if (!sr.isSent()) {
				throw new RuntimeException("Sending of request failed.");
			}

			byte[] response = null;
			synchronized (requestLock) {
				try {
					if (operationTimeout > 0) {
						long elapsedTime = System.currentTimeMillis() - operationStart;
						while ((elapsedTime < operationTimeout) && (receivedResponse == null)) {
							requestLock.wait(operationTimeout - elapsedTime);
							elapsedTime = System.currentTimeMillis() - operationStart;
						}
					} else {
						while (receivedResponse == null) {
							requestLock.wait();
						}
						requestLock.wait();
					}
				} catch (InterruptedException e) {
					throw new RuntimeException("Request interrupted.", e);
				}

				response = receivedResponse;
				pendingRequestTag = -1;
			}

			return response;
		} finally {
			synchronized (requestLock) {
				pendingRequestTag = -1;
				receivedResponse = null;
			}
		}
	}

	/**
	 * Handles a received message.
	 * 
	 * @param tag
	 *            the tag associated with the received message.
	 * @param message
	 *            the message content.
	 */
	private void handleMessage(int tag, byte[] message) {
		synchronized (requestLock) {
			if ((tag >= 0) && (tag == pendingRequestTag)) {
				receivedResponse = message;
			}

			requestLock.notifyAll();
		}
	}

	/**
	 * Encodes a numeric value.
	 * 
	 * @param value
	 *            the value
	 * @return the value encoded as variable-length sequence of bytes.
	 */
	private static byte[] encodeNumber(int value) {
		if (value == Integer.MIN_VALUE) {
			return new byte[] { (byte) 0x40 };
		}

		boolean negativeValue = value < 0;
		value = Math.abs(value);

		// Decompose value
		int[] buffer = new int[5];
		int length = 0;
		while (value > 63) {
			buffer[length] = value % 128;
			value = value / 128;
			length++;
		}
		buffer[length] = value;
		length++;

		// Update sign bit
		if (negativeValue) {
			buffer[length - 1] = buffer[length - 1] | 0x40;
		}

		// Revert bytes and add "next byte" flags
		byte[] result = new byte[length];
		for (int i = length - 1, j = 0; i > 0; i--, j++) {
			result[j] = (byte) (buffer[i] | 0x80);
		}
		result[length - 1] = (byte) buffer[0];

		return result;
	}

	/**
	 * Decodes a numeric value.
	 * 
	 * @param data
	 *            the array of bytes.
	 * @param offset
	 *            the offset in data array where encoded numeric value starts.
	 * @return the decoded value.
	 */
	private static int decodeNumber(byte[] data, int offset) {
		try {
			int aByte = data[offset] & 0xFF;
			boolean negativeValue = ((aByte & 0x40) != 0);
			boolean nextByte = ((aByte & 0x80) != 0);
			int result = (byte) (aByte & 0x3F);

			// Special handling for MIN_VALUE
			if (!nextByte && negativeValue && (result == 0)) {
				return Integer.MIN_VALUE;
			}

			// Decoding
			while (nextByte) {
				offset++;
				aByte = data[offset] & 0xFF;
				result = result * 128 + (aByte & 0x7F);
				nextByte = ((aByte & 0x80) != 0);
			}

			return negativeValue ? -result : result;
		} catch (Exception e) {
			throw new RuntimeException("Invalid message format.", e);
		}
	}
}
