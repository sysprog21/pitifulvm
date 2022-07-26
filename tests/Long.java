public class Long
{
    public static long reutrnLong(long x) {
        return x;
    }
    public static void main(final String[] array) {
        final long n = java.lang.Long.MAX_VALUE;
        final long n2 = 2000L;
        System.out.println(reutrnLong(n));
        System.out.println(n + n2);
        System.out.println(n - n2);
        System.out.println(n * n2);
        System.out.println(n / n2);
        System.out.println(n % n2);
        System.out.println((int)n + (long)(int)n2);
    }
}