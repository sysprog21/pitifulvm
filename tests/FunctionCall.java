public class FunctionCall {
    public static void main(String[] args) {
        System.out.println(add(2, 200) + add(1, 200));
    }

    public static int add(int a, int b) {
        return a + b;
    }
}
