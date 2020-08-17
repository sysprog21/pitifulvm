// Taken from https://projecteuler.net/problem=31
public class CoinSums {
    public static void main(String[] args) {
        System.out.println(waysToMake(200, 200));
    }

    public static int waysToMake(int target, int maxCoin) {
        if (maxCoin == 1) return 1;

        int nextCoin = maxCoin == 5 || maxCoin == 50
                       ? maxCoin * 2 / 5
                       : maxCoin / 2;
        int ways = 0;
        while (target >= 0) {
            ways += waysToMake(target, nextCoin);
            target -= maxCoin;
        }
        return ways;
    }
}
