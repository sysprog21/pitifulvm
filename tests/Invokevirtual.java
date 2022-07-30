class InvokevirtualA {
    void print(int val) {
        System.out.println(val);
    }
}

class InvokevirtualB {
    void print(InvokevirtualA obj, int val) {
        obj.print(val);
    }
}

class Invokevirtual {
    int a;
    public void print() {
        System.out.println(a);
    }
    public static void main(String args[]) {
        Invokevirtual x = new Invokevirtual();
        InvokevirtualA obj1 = new InvokevirtualA();
        InvokevirtualB obj2 = new InvokevirtualB();
        x.a = 3;
        x.print();
        obj1.print(2);
        obj2.print(obj1, 3);
    }
}