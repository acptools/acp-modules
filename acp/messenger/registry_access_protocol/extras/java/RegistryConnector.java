package net.acprog.modules.messenger;

/**
 * Interface to access a remote registry.
 */
public interface RegistryConnector {

	/**
	 * Reads a value from a register.
	 * 
	 * @param registerId
	 *            the identifier of the register.
	 * @return the value of register
	 * @throws RuntimeException
	 *             if the operation failed.
	 */
	public int readRegister(int registerId) throws RuntimeException;

	/**
	 * Writes a value to register.
	 * 
	 * @param registerId
	 *            the identifier of the register.
	 * @param value
	 *            the value to be written to the register.
	 * @throws RuntimeException
	 *             if the operation failed.
	 */
	public void writeRegister(int registerId, int value) throws RuntimeException;

}
