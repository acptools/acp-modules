package net.acprog.modules.messenger;

import java.util.Scanner;

//Requires JSSC: https://github.com/scream3r/java-simple-serial-connector/releases
import net.acprog.modules.messenger.GEPMessenger.MessageListener;
import net.acprog.modules.messenger.GEPMessenger.SendRequest;

public class GEPTest {

	public static void main(String[] args) {
		// Create messenger with listener for receiving messages.
		GEPMessenger messenger = new GEPMessenger("COM9", 9600, 100, new MessageListener() {
			@Override
			public void onMessageReceived(int tag, byte[] message) {
				if (tag >= 0) {
					System.out.println("Message received (tag " + tag + "): ");
				} else {
					System.out.println("Message received: ");
				}
				System.out.println(new String(message));
			}
		});

		// Start messenger.
		messenger.start();

		// Send lines as messages to the messenger.
		Scanner s = new Scanner(System.in);
		int tagCounter = 1;
		while (s.hasNextLine()) {
			String line = s.nextLine();
			SendRequest sr = messenger.sendMessage(line.getBytes(), tagCounter);
			tagCounter++;

			// Optional code that waits for completion of the message send.
			try {
				sr.waitForCompletion();
			} catch (InterruptedException ignore) {

			}
		}
		s.close();
	}

}
