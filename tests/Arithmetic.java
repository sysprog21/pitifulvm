public class Arithmetic {
    public static void main(String[] args) {
        System.out.println(add(0, 0));
        System.out.println(add(0, 1));
        System.out.println(add(1, 0));
        System.out.println(add(2, 3));
        System.out.println(add(3, 2));
        System.out.println(add(10, -100));
        System.out.println(add(-100, 10));
        System.out.println(add(1000, -100));
        System.out.println(add(-100, 1000));
        System.out.println(add(-123, -456));
        System.out.println(add(-456, -123));
        System.out.println(add(1 << 30 | 1, 1 << 30 | 2));
        System.out.println(add(1 << 31 | 1, 1 << 31 | 2));

        System.out.println(sub(0, 0));
        System.out.println(sub(0, 1));
        System.out.println(sub(1, 0));
        System.out.println(sub(3, 10));
        System.out.println(sub(10, 3));
        System.out.println(sub(10, -100));
        System.out.println(sub(-100, 10));
        System.out.println(sub(1000, -100));
        System.out.println(sub(-100, 1000));
        System.out.println(sub(-123, -456));
        System.out.println(sub(-456, -123));
        System.out.println(sub((1 << 31) - 1, 1 << 31));
        System.out.println(sub(0, 1 << 31));

        System.out.println(mul(0, 0));
        System.out.println(mul(0, 100));
        System.out.println(mul(100, 0));
        System.out.println(mul(12, 34));
        System.out.println(mul(10, -11));
        System.out.println(mul(-10, 11));
        System.out.println(mul(-100, -100));
        System.out.println(mul(1, 1 << 31));
        System.out.println(mul(1, (1 << 31) - 1));
        System.out.println(mul(1 << 31, -1));
        System.out.println(mul(123456, 987654));
        System.out.println(mul(-123456, 987654));
        System.out.println(mul(123456, -987654));
        System.out.println(mul(-123456, -987654));

        System.out.println(div(0, 1));
        System.out.println(div(10, 1));
        System.out.println(div(20, 3));
        System.out.println(div(23, 24));
        System.out.println(div(100, 20));
        System.out.println(div(-100, 20));
        System.out.println(div(100, -20));
        System.out.println(div(-100, -20));
        System.out.println(div(100, 3));
        System.out.println(div(-100, 3));
        System.out.println(div(100, -3));
        System.out.println(div(-100, -3));

        System.out.println(rem(0, 1));
        System.out.println(rem(10, 1));
        System.out.println(rem(20, 3));
        System.out.println(rem(23, 24));
        System.out.println(rem(100, 20));
        System.out.println(rem(-100, 20));
        System.out.println(rem(100, -20));
        System.out.println(rem(-100, -20));
        System.out.println(rem(100, 3));
        System.out.println(rem(-100, 3));
        System.out.println(rem(100, -3));
        System.out.println(rem(-100, -3));

        System.out.println(neg(0));
        System.out.println(neg(1));
        System.out.println(neg(-10));
        System.out.println(neg(100));
        System.out.println(neg(-1000));
        System.out.println(neg((1 << 31) - 1));
        System.out.println(neg(1 << 31));
    }

    public static int add(int a, int b) {
        return a + b;
    }
    public static int sub(int a, int b) {
        return a - b;
    }
    public static int mul(int a, int b) {
        return a * b;
    }
    public static int div(int a, int b) {
        return a / b;
    }
    public static int rem(int a, int b) {
        return a % b;
    }
    public static int neg(int a) {
        return -a;
    }
}
