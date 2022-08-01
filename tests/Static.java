public class Static {
    static int x = 1, y = 2, z = 3;
    static StaticA obj;
    static {
        StaticA.x = 3;
        System.out.println(2);
        System.out.println(StaticA.x);
    }
    public static void main(String[] args) {
        StaticB.x = 4;
        System.out.println(Static.x);
        System.out.println(Static.y);
        System.out.println(Static.z);
        System.out.println(StaticA.x);
        System.out.println(StaticB.x);
    }
}

class StaticA {
    static int x;
}

class StaticB {
    static int x;
} 