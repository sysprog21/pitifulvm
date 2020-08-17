// Taken from https://projecteuler.net/problem=46
public class Goldbach {
    public static void main(String[] args) {
        System.out.println(smallestCounterexample());
    }

    public static int smallestCounterexample() {
        testExample: for (int test = 5; ; test += 2) {
            for (int squareRoot = 0; ; squareRoot++) {
                int rest = test - squareRoot * squareRoot * 2;
                if (rest <= 0) {
                    return test;
                }
                if (isPrime(rest)) {
                    continue testExample;
                }
            }
        }
    }
    public static boolean isPrime(int n) {
        for (int test = 2; test * test <= n; test++) {
            if (n % test == 0) {
                return false;
            }
        }
        return true;
    }
}
