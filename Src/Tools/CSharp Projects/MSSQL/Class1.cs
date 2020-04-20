using System;
using System.Data.SqlClient;
using System.IO;
using System.Data;

namespace C3MSSQL
{
    public class Class1
    {
        public static int OnSendToChannel(string packet, string servername, string databasename, string tablename, string username, string password, string outboundID)
        {
            byte[] raw = Convert.FromBase64String(packet);
            string connectionString = "";
            if (username.Contains("\\"))
            {
                connectionString = String.Format("Server={0};Database={1};Integrated Security=SSPI;User Id={2};Password={3}", servername, databasename, username, password);
            }
            else
            {
                connectionString = String.Format("Server={0};Database={1};User Id={2};Password={3}", servername, databasename, username, password);
            }

            try
            {
                SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder(connectionString);
                using (SqlConnection openCon = new SqlConnection(builder.ConnectionString))
                {
                    openCon.Open();
                    string conn = String.Format("INSERT into dbo.{0} (MSG) VALUES ('{1}');", tablename, outboundID + packet);
                    SqlCommand comm = openCon.CreateCommand();
                    comm.CommandText = conn;
                    if (comm.ExecuteNonQuery() == 0)
                    {
                        return 0;
                    }
                    else
                    {
                        return raw.Length;
                    }
                }
            }
            catch (Exception e)
            {
                return 0;
            }
        }
        public static string OnReceiveFromChannel(string servername, string databasename, string tablename, string username, string password, string inboundID)
        {
            string packetData = "";
            int messageID = 0;
            string connectionString = "";

            if (username.Contains("\\"))
            {
                connectionString = String.Format("Server={0};Database={1};Integrated Security=SSPI;User Id={2};Password={3}", servername, databasename, username, password);
            }
            else
            {
                connectionString = String.Format("Server={0};Database={1};User Id={2};Password={3}", servername, databasename, username, password);
            }

            try
            {
                SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder(connectionString);
                using (SqlConnection openCon = new SqlConnection(builder.ConnectionString))
                {                    
                    openCon.Open();
                    string conn = String.Format("SELECT TOP 1 * FROM dbo.{0} WHERE MSG like '{1}%';", tablename, inboundID);
                    SqlCommand comm = openCon.CreateCommand();
                    comm.CommandText = conn;
                    using (SqlDataReader rd = comm.ExecuteReader())
                    {
                        if (rd.Read())
                        {
                            packetData = rd["MSG"].ToString();
                            messageID = int.Parse(rd["ID"].ToString());
                        }
                    }
                    conn = String.Format("DELETE FROM dbo.{0} WHERE ID = {1};", tablename, messageID);
                    comm.CommandType = CommandType.Text;
                    comm.CommandText = conn;
                    comm.ExecuteNonQuery();
                }
            }
            catch (Exception e)
            {
                //File.AppendAllText(@"C:\Temp\errrecv.txt", e.Message + "\r\n");
            }
            //Get rid of inboundID from packet and return.
            return TrimStart(packetData, inboundID);
        }
        public static void PrepareTable(string servername, string databasename, string tablename, string username, string password)
        {
            string connectionString = "";
            if (username.Contains("\\"))
            {
                connectionString = String.Format("Server={0};Database={1};Integrated Security=SSPI;User Id={2};Password={3}", servername, databasename, username, password);
            }
            else
            {
                connectionString = String.Format("Server={0};Database={1};User Id={2};Password={3}", servername, databasename, username, password);
            }
            try
            {
                if(!checkifexists(tablename, connectionString))
                {
                    try
                    {
                        using (SqlConnection openCon = new SqlConnection(connectionString))
                        {
                            openCon.Open();
                            string conn = String.Format("CREATE TABLE dbo.{0} (ID INT IDENTITY(1,1) NOT NULL PRIMARY KEY, MSG varchar(max));", tablename);
                            SqlCommand comm = openCon.CreateCommand();
                            comm.CommandText = conn;
                            SqlDataReader rd = comm.ExecuteReader();
                        }
                    }
                    catch (Exception e)
                    {
                        //File.AppendAllText(@"C:\Temp\PrepareTable.txt", e.Message + "\r\n");
                        return;
                    }
                }
            }
            catch(Exception e)
            {
                //File.AppendAllText(@"C:\Temp\PrepareTable2.txt", e.Message + "\r\n");
            }
        }
        public static void ClearTable(string servername, string databasename, string tablename, string username, string password)
        {
            string connectionString = "";
            if (username.Contains("\\"))
            {
                connectionString = String.Format("Server={0};Database={1};Integrated Security=SSPI;User Id={2};Password={3}", servername, databasename, username, password);
            }
            else
            {
                connectionString = String.Format("Server={0};Database={1};User Id={2};Password={3}", servername, databasename, username, password);
            }
            try
            {
                if (!checkifexists(tablename, connectionString))
                {
                    try
                    {
                        using (SqlConnection openCon = new SqlConnection(connectionString))
                        {
                            openCon.Open();
                            String conn = String.Format("DELETE FROM dbo.{0};", tablename);
                            SqlCommand comm = openCon.CreateCommand();
                            comm.CommandText = conn;
                            SqlDataReader rd = comm.ExecuteReader();
                        }
                    }
                    catch (Exception e)
                    {
                        //File.AppendAllText(@"C:\Temp\ClearTable.txt", e.Message + "\r\n");
                        return;
                    }
                }
            }
            catch (Exception e)
            {
                //File.AppendAllText(@"C:\Temp\ClearTable2.txt", e.Message + "\r\n");
            }
        }
        public static bool checkifexists(string tablename, string connectionString)
        {
            try
            {
                using (SqlConnection openCon = new SqlConnection(connectionString))
                {
                    openCon.Open();
                    string conn = String.Format("Select * FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME = '{0}'", tablename);
                    SqlCommand comm = openCon.CreateCommand();
                    comm.CommandText = conn;
                    using (SqlDataReader rd = comm.ExecuteReader())
                    {
                        if (rd.Read())
                        {
                            return rd.HasRows;
                        }
                    }
                    return false;
                }
            }
            catch
            {
                return false;
            }
        }
        public static string TrimStart(string target, string trimString)
        {
            if (string.IsNullOrEmpty(trimString)) return target;

            string result = target;
            while (result.StartsWith(trimString))
            {
                result = result.Substring(trimString.Length);
            }

            return result;
        }
    }
}
