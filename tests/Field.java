class Field {
    Field(int a, int b, int c) {
        x = a;
        y = b;
        z = c;
    }
    public static void main(String[] args) {
        Field f = new Field(1, 2, 3);
        System.out.println(f.x + f.y + f.z);
    }
    int x, y, z;
}