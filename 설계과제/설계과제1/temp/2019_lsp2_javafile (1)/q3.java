import java.io.File;
import java.io.IOException;
import java.io.FileWriter;

public class q3{
	public static void main(String[] args) throws IOException{

		
		File file = new File("q3java.txt");

		/****** 두번째 매개변수 ******/
		/****** true : 기존 파일의 내용 이후부터 쓰여짐 ******/
		/****** false : 처음부터 쓰여짐 ******/

		FileWriter writer = new FileWriter(file, false);

		writer.write("2019 OSLAB\n");
		writer.write("Linux System Programming\n");

		writer.flush();
		System.out.printf("DONE\n");

		if(writer != null)
			writer.close();
		
	}
}
