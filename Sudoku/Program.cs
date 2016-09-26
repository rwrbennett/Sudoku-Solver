using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;


namespace Sudoku
{
    

    static class Program
    {
        //[DllImport("sample2.dll")]

        //public static extern void Solve(byte[] buffer);
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            
            

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Form1 mainForm = new Form1();
            Application.Run(mainForm);
            
            
        }
    }
}
