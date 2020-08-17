/* Taken from https://projecteuler.net/problem=14
 * (Modified slightly so numbers fit in a 32-bit int.) */
public class Collatz {
    public static void main(String[] args) {
        int longestStart = 0;
        int longestLength = 0;
        for (int initial = 1; initial < 100_000; initial++) {
            int length = 0;
            int current = initial;
            while (current > 1) {
                length++;
                current = current % 2 == 0 ? current / 2 : current * 3 + 1;
            }
            if (length > longestLength) {
                longestStart = initial;
                longestLength = length;
            }
        }
        System.out.println(longestStart);
    }
}
