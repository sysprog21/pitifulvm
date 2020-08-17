/* Taken from https://projecteuler.net/problem=24
 * (Modified slightly so numbers fit in a 32-bit int.) */
public class DigitPermutations {
    public static void main(String[] args) {
        int digits = 876543210;
        int permutationsToSkip = 99_999;
        for (int remainingDigits = 8; remainingDigits >= 0; remainingDigits--) {
            int possibilities = factorial(remainingDigits);
            int index = permutationsToSkip / possibilities;
            permutationsToSkip -= index * possibilities;
            System.out.println(getDigit(digits, index));
            digits = removeDigit(digits, index);
        }
    }

    public static int factorial(int n) {
        int product = 1;
        while (n > 0) {
            product *= n--;
        }
        return product;
    }

    public static int getDigit(int digits, int index) {
        while (index > 0) {
            digits /= 10;
            index--;
        }
        return digits % 10;
    }

    public static int removeDigit(int digits, int index) {
        int powerOfTen = 1;
        while (index > 0) {
            powerOfTen *= 10;
            index--;
        }
        return digits / powerOfTen / 10 * powerOfTen + digits % powerOfTen;
    }
}
