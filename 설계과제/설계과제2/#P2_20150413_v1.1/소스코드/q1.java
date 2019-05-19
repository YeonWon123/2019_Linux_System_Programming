import java.util.Scanner;

public class q1{
	public static void main(String[] args){
		Scanner scn = new Scanner(System.in);

		System.out.printf("Enter the number : ");

		int num;
		num = scn.nextInt();
		int even=0, odd=0;

		for(int i=1; i<=num; i++){ // Checking...
			if(i % 2 == 0){
				even+=i;
			}
			else{
				odd+=i;
			}
		}

		System.out.printf("Sum of Even number : %d\n", even);
		System.out.printf("Sum of Odd number : %d\n", odd);

		return ;
	}
}
