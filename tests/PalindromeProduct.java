// Taken from https://projecteuler.net/problem=4
public class PalindromeProduct {
    public static void main(String[] args) {
        int maxPalindrome = 0;
        for (int i = 100; i <= 999; i++) {
            for (int j = 100; j <= 999; j++) {
                int product = i * j;
                if (product > maxPalindrome && isPalindrome(product)) {
                    maxPalindrome = product;
                }
            }
        }
        System.out.println(maxPalindrome);
    }

    public static int reverse(int n) {
        int reversed = 0;
        while (n != 0) {
            reversed = reversed * 10 + n % 10;
            n /= 10;
        }
        return reversed;
    }
    public static boolean isPalindrome(int n) {
        return n == reverse(n);
    }
}
