import java.util.Locale;
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class Updater extends Frame 
{
    private Font font = new Font("serif", Font.ITALIC+Font.BOLD, 36);

    public Updater() 
    {
        super("");
        
	WindowAdpt WAdapter = new WindowAdpt();      
	addWindowListener(WAdapter);

	setTitle("Codename Twilight updater");
        setLayout(null);
 		
        setVisible(true);
        setSize(550, 400);
	setResizable(false);
    }
	
    public void handleQuit()
    {
        System.exit(0);
    }
	
    class WindowAdpt extends java.awt.event.WindowAdapter 
    {
        public void windowClosing(java.awt.event.WindowEvent event) 
	{
            handleQuit();
        }
    }
    
    public static void main(String args[]) 
    {
        new Updater();
    }
}
