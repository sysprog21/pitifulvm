public class Static {
    static int x, y, z;
    public static void main(String[] args) {
        x = 3;
        y = 4;
        z = 5;
        StaticA.x = 1;
        StaticB.x = Static.x + StaticA.x;
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