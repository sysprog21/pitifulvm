public class Initializer {
    static {
        System.out.println(1);
    }
    Initializer () {
        System.out.println(2);
    }
    public static void call() {

    }
    public static void main(String[] args) {
        /* only call `Initializer` init */
        InitializerB.call();
        /* call remaining init in order */
        InitializerB obj = new InitializerB();
    }
}

class InitializerA extends Initializer {
    static {
        System.out.println(3);
    }
    InitializerA() {
        super();
        System.out.println(4);
    }
}

class InitializerB extends InitializerA {
    static {
        System.out.println(5);
    }
    InitializerB() {
        super();
        System.out.println(6);
    }
}