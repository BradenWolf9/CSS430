import java.net.*;
import java.io.*;

public class Worker implements Runnable
{
  private Socket myClient;

  /* recieve client and set it to my client */
  Worker(Socket client) {myClient = client;}

  public void run()
  {
    try
    {
      /* create PrintWriter object to write to clinets ostream */
      PrintWriter pout = new PrintWriter(myClient.getOutputStream(), true);

      /* write the Date to the socket */
      pout.println(new java.util.Date().toString());

      /* close the socket and resume listening for connections */
      myClient.close();

    }
    catch(IOException e)
    {
      System.err.println(e);
    }
  }
}
