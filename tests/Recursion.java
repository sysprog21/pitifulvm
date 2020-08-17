public class Recursion {
    public static void main(String[] args) {
        printFactorial(10);
        System.out.println(fib(20));
        System.out.println(isEven(1000) ? 1 : 0);
        System.out.println(isOdd(1000) ? 1 : 0);
        System.out.println(isEven(1001) ? 1 : 0);
        System.out.println(isOdd(1001) ? 1 : 0);
    }

    public static int factorial(int n) {
        if (n == 0) {
            return 1;
        }
        return n * factorial(n - 1);
    }
    public static void printFactorial(int n) {
        System.out.println(factorial(n));
    }

    public static int fib(int n) {
        return n < 2 ? n : fib(n - 2) + fib(n - 1);
    }

    public static boolean isEven(int n) {
        return n == 0 || isOdd(n - 1);
    }

    public static boolean isOdd(int n) {
        return n != 0 && isEven(n - 1);
    }
}
