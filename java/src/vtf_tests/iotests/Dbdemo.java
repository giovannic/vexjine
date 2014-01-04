import java.sql.*;

public class Dbdemo {


	private boolean connected;	
	public Dbdemo() {
		connected = false;
	}


	public static void main(String[] args) {

		String dbClassName = "com.mysql.jdbc.Driver";
		String CONNECTION = "jdbc:mysql://localhost/tpcwdb?user=nb605&password=admin&useReadAheadInput=false";
		int iterations = Integer.parseInt(args[0]);


		try {
			Connection c = java.sql.DriverManager.getConnection(CONNECTION);
	
			String s = new String("SELECT * FROM address WHERE addr_city LIKE '%"+args[1]+"%'");
//			String s = new String("select * from shopping_cart");

			for (int i = 0; i<iterations; ++i) {
				PreparedStatement get_name = c.prepareStatement(s);
				ResultSet rs = get_name.executeQuery();
				rs.close();
			}

			c.close();	
		} catch (java.lang.Exception ex) {
			ex.printStackTrace();
		}

	
	}

}

