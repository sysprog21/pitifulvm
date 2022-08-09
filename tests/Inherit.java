public class Inherit {
    static int x = 1;
    int y = 5;

    public static void static_call() {
        System.out.println(1);
    }

    public void virtual_call() {
        System.out.println(2);
    }

    public static void main(String[] args) {
        Inherit obj = new Inherit();
        InheritA objA = new InheritA();
        InheritB objB = new InheritB();

        /* check shared static fields */
        System.out.println(obj.x);
        System.out.println(objA.x);
        System.out.println(objB.x);
        obj.x = 2;
        System.out.println(obj.x);
        System.out.println(objA.x);
        System.out.println(objB.x);
        objA.x = 3;
        System.out.println(obj.x);
        System.out.println(objA.x);
        System.out.println(objB.x);

        /* check fields */
        System.out.println(obj.y);
        System.out.println(objA.y);
        System.out.println(objB.y);
        obj.y = 2;
        System.out.println(obj.y);
        System.out.println(objA.y);
        System.out.println(objB.y);
        objA.y = 3;
        objB.y = 5;
        System.out.println(obj.y);
        System.out.println(objA.y);
        System.out.println(objB.y);

        /* check static methods inheritance (compiler will replace objects with classes) */
        obj.static_call();
        objA.static_call();
        objB.static_call();

        /* check virtual methods inheritance */
        obj.virtual_call();
        objA.virtual_call();
        objB.virtual_call();
    }
}

class InheritA extends Inherit {
    
}

class InheritB extends InheritA {
    /* check override */
    public void virtual_call() {
        System.out.println(3);
    }
    public static void static_call() {
        System.out.println(4);
    }
}