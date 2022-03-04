import java.net.*;
import java.io.*;

public class DateServerMT
{
  public static void main(String[] args) {
    try {
      /* create new socket for server */
      ServerSocket sock = new ServerSocket(Integer.parseInt(args[0], 10));

      /* now listen for connections */
      while (true) {
        /* block to accept client */
        Socket client = sock.accept();

        /* create new worker object */
        Worker myWorker = new Worker(client);

        /* create new thread and set it to run worker */
        Thread myThread = new Thread(myWorker);

        /* start thread */
        myThread.start();
      }
    }
    catch (IOException ioe) {
      System.err.println(ioe);
    }
  }
}
