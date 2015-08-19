package net.acprog.modules.messenger;

import java.util.Scanner;

public class RegistryTest {

	public static void main(String[] args) throws InterruptedException {
		GEPRegistryConnector connector = new GEPRegistryConnector("COM9", 33600);
		connector.start();
		System.out.println("Initial sleep 1 second");
		Thread.sleep(1000);

		System.out.println("Write your commands (exit, write register value, read register):");
		try (Scanner scanner = new Scanner(System.in)) {
			commandLoop: while (scanner.hasNextLine()) {
				String line = scanner.nextLine().trim();
				if (line.isEmpty()) {
					continue;
				}

				try (Scanner commandScanner = new Scanner(line)) {
					switch (commandScanner.next()) {
					case "exit":
						break commandLoop;
					case "write":
						connector.writeRegister(commandScanner.nextInt(), commandScanner.nextInt());
						System.out.println("Written.");
						break;
					case "read":
						System.out.println("Value: " + connector.readRegister(commandScanner.nextInt()));
						break;
					}
				} catch (Exception e) {
					System.out.println("Command failed.");
				}
			}
		}
	}
}
