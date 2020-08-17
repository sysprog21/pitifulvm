public class IntegerTypes {
    public static void main(String[] args) {
        int digits = 0;
        digits = addDigit(digits, '1');
        digits = addDigit(digits, '2');
        digits = addDigit(digits, '3');
        digits = addDigit(digits, '4');
        System.out.println(digits);

        System.out.println(0x12ABCDEF);
        System.out.println(intFromBytes((byte) 0x12, (byte) 0xAB, (byte) 0xCD, (byte) 0xEF));

        System.out.println(0xAB);
        System.out.println(0xCD);
        System.out.println(highByte((short) 0xABCD));
        System.out.println(lowByte((short) 0xABCD));
    }

    public static int addDigit(int digits, char digit) {
        return digits * 10 + digit - '0';
    }

    public static int signedByte(byte b) {
        return (b + 0x100) % 0x100;
    }

    public static int intFromBytes(byte b1, byte b2, byte b3, byte b4) {
        return b1 * 0x100_00_00 +
               signedByte(b2) * 0x100_00 +
               signedByte(b3) * 0x100 +
               signedByte(b4);
    }

    public static int highByte(short s) {
        return (s + 0x100_00) % 0x100_00 / 0x100;
    }

    public static int lowByte(short s) {
        return (s % 0x100 + 0x100) % 0x100;
    }
}
