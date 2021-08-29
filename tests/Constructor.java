class Constructor {
    Constructor(int x, int y, int z) {
        System.out.println(x);
        System.out.println(y);
        System.out.println(z);
    }
    static void static_call(int x) {
        System.out.println(x);
    }
    public static void main(String[] args) {
        Constructor c = new Constructor(5, 6, 7);
        Constructor_A a1 = new Constructor_A(10, 11, 12);
        Constructor_B b1 = new Constructor_B(8, 9);
        Constructor_C c1 = new Constructor_C(2);
        c.static_call(18);
    }
}

class Constructor_A extends Constructor {
    Constructor_A(int x, int y, int z) {
        super(x, y, z);
        System.out.println(x);
        System.out.println(y);
        System.out.println(z);
    }
}

class Constructor_B extends Constructor_A {
    Constructor_B(int x, int y) {
        super(x, y, 2);
        System.out.println(x);
        System.out.println(y);
    }
}

class Constructor_C extends Constructor_B {
    Constructor_C(int x)  {
        super(x, 1);
        System.out.println(x);
    }
}