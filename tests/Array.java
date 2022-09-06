public class Array {
    static int iarr[] = new int[5];
    static long larr[][] = new long[2][3];
    static ArrayA rarr[][][] = new ArrayA[4][2][3];
    public static void main(String[] args) {
        /* test default value */
        System.out.println(iarr[0]);
        System.out.println(larr[1][2]);
        System.out.println(rarr[2][1][1]);

        iarr[2] = 3;
        larr[0][1] = 5;
        rarr[1][1][1] = new ArrayA();
        System.out.println(iarr[2]);
        System.out.println(larr[0][1]);
        System.out.println(rarr[1][1][1].x);
    }
}

class ArrayA {
    int x = 5;
}