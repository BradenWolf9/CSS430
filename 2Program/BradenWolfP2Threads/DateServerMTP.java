import java.net.*;
import java.io.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;

public class DateServerMTP
{
  public static void main(String[] args) {
    try {
      /* create new socket for server */
      ServerSocket sock = new ServerSocket(Integer.parseInt(args[0], 10));

      /* create new thread pool */
      ExecutorService threadExecutor = Executors.newCachedThreadPool();

      /* now listen for connections */
      while (true) {
        /* block to accept client */
        Socket client = sock.accept();

        /* create new worker object */
        Worker myWorker = new Worker(client);

        /* add thread to pool */
        threadExecutor.execute(myWorker);
      }
    }
    catch (IOException ioe) {
      System.err.println(ioe);
    }
  }
}
