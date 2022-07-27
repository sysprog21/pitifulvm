public class Caller {
    public static void main(String[] args) {
        Callee.func();
    }
}

class Callee {
    public static void func() {
        System.out.println(1);
    }
}
