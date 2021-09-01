public class Strings {
    public static String f(String x) {
        return x + " abc " + x;
    }
    public static void main(String args[])
    {
        String str1 = "Hello";
        String str2 = " Jvm ";
        String str3 = "string";
        String str4 = "x\bx\tx\\x\nx\1x\2x\3x\4x\5x\6x";
        int x = 100;
        long y = java.lang.Long.MAX_VALUE;
        /* test string argument */
        System.out.println(f("test"));
        /* test string concat */
        System.out.println(str1 + " constant " + str2);
        System.out.println(str1 + str2 + str3 + str4);
        /* test string concat with number */
        System.out.println("1" + 2 + x + "3" + y + 0.8 + str2);
        /* test invokedynamic arguments */
        System.out.println("prefix \1" + str1 + "suffix \2");
    }
}