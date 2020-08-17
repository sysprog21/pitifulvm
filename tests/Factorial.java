public class Factorial
{
    public static void main(String[] args)
    {
        int NUM_FACTS = 12;
        int test = 0;
        for (int i = 0; i < NUM_FACTS; i++)
            test += factorial(i);
        System.out.println(test);
    }

    public static int factorial(int n)
    {   int result = 1;
        for (int i = 2; i <= n; i++)
            result *= i;
        return result;
    }
}
