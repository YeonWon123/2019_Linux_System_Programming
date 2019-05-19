class Stack{

	int top;
	int[] stack;

	public static final int STACK_SIZE = 10;

	public Stack(){
		top = -1;
		stack = new int[STACK_SIZE];
	}

	public int peek(){
		return stack[top];
	}

	public void push(int value){
		stack[++top] = value;
		System.out.printf(stack[top] + " PUSH !\n");
	}

	public int pop(){
		System.out.printf(stack[top] + " POP !\n");
		return stack[top--];
	}

	public void printStack(){
		System.out.printf("\n-----STACK LIST-----\n");

		for(int i=top; i>=0; i--){
			System.out.printf("%d\n",stack[i]);
		}

		System.out.printf("-----END OF LIST-----\n");
	}
}

public class q2{

	public static void main(String args[]){
		
		Stack st = new Stack();

		st.push(5);
		st.push(2);
		st.push(3);
		st.push(4);
		st.push(1);

		st.printStack();

		st.pop();
		st.pop();
		st.push(15);

		System.out.printf("TOP OF STACK : %d\n", st.peek());

		st.printStack();

		st.pop();
		st.pop();
		st.pop();
		st.pop();

		st.push(30);

		st.printStack();
	}
}
