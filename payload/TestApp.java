import java.io.*;
import java.util.*;

class TestApp 
{
    static String outPath = "/Windows/Temp/prl_host_out.txt";
    
    // reserve 30 * 4 characters for command to execute
    static String defaultCmd = "***command_string***          " +
                               "                              " +
                               "                              " +
                               "                              ";

    public static int exec(String command, StringBuilder output)
    {
        int ret = -1;

        try
        {            
            Runtime r = Runtime.getRuntime();
            Process p = r.exec(command);

            BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
            String s = "";

            // read the output from the command
            while ((s = stdInput.readLine()) != null) 
            {
                output.append(s + "\n");
            }

            ret = p.waitFor();
            System.out.println("Process exit code: " + ret);            
        } 
        catch (Throwable e)
        {
            e.printStackTrace();
        }

        return ret;
    }

    static void write(String path, String data) throws IOException
    {
        Writer writer = new PrintWriter(path);

        writer.write(data);
        writer.close();
    }

    public static void main(String[] args) 
    { 
        if (defaultCmd.substring(0, 3).equals("***"))
        {
            System.out.println("Command is not specified");    
            return;
        }

        StringBuilder output = new StringBuilder();
        if (exec(defaultCmd, output) == -1)
        {
            output.append("Error while executing command");
        }
                     
        String volumesPath = "/Volumes";
        File folder = new File(volumesPath);

        for (File file : folder.listFiles()) 
        {
            if (file.isDirectory()) 
            {                
                String outFile = volumesPath + "/" + file.getName() + outPath;

                try
                {
                    // save output into the file
                    write(outFile, output.toString());

                    System.out.println("Creating " + outFile);
                }                    
                catch (IOException e)
                {
                    continue;       
                }                
            }
        }
    }
}
