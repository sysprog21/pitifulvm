public class Primes {
    public static void main(String[] args) {
        printPrimes(10000);
    }

    public static void printPrimes(int max) {
        for (int n = 0; n < max; n++) {
            if (isPrime(n)) {
                System.out.println(n);
            }
        }
    }

    public static boolean isPrime(int n) {
        if (n < 2) {
            return false;
        }

        for (int testFactor = 2; testFactor < n; testFactor++) {
            if (n % testFactor == 0) {
                return false;
            }
        }

        return true;
    }
}
