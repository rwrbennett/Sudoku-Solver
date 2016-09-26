using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;

namespace Sudoku
{
    public partial class Form1 : Form
    {
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate void ProgressCallback(IntPtr pArray);

        [DllImport("sudoku_solver.dll", EntryPoint = "solve", CallingConvention = CallingConvention.Cdecl)]
        static extern void solve(IntPtr pArray, int nSize);

        [DllImport("sudoku_solver.dll", EntryPoint = "showSteps", CallingConvention = CallingConvention.Cdecl)]        
        static extern void showSteps(IntPtr pArray, int nSize, [MarshalAs(UnmanagedType.FunctionPtr)] ProgressCallback callbackPointer);

        int size;
        IntPtr pnt;
        byte[] array = new byte[81];
        bool inProgress;

        private void onCallback(IntPtr pArray)
        {
            byte[] array2 = new byte[array.Length];
            
            Marshal.Copy(pArray, array2, 0, array.Length);
            setValues(array2);
            Application.DoEvents();

        }

        public Form1()
        {

            InitializeComponent();
            inProgress = false;

            Padding p = new Padding(8,2,1,1);
            foreach (DataGridViewColumn c in dataGridView1.Columns)
            {
                c.DefaultCellStyle.Padding = p;
                c.DefaultCellStyle.Font = new Font("Arial", 24F, GraphicsUnit.Pixel);
            }
            dataGridView1.Rows.Add(9);
            for (int i = 0; i < 9; i++)
            {
                dataGridView1.Columns[i].Width = 40;
                dataGridView1.Rows[i].Height = 40;
            }
            
            
        }

        public void setValues(byte[] squareArray)
        {
            for (int i = 0; i < 9; i++)
            {
                for (int j = 0; j < 9; j++)
                {
                    if (squareArray[(i*9)+j]>0)
                        dataGridView1.Rows[i].Cells[j].Value = squareArray[(i*9)+j];
                    else
                        dataGridView1.Rows[i].Cells[j].Value = "";                    
                }
            }
            dataGridView1.Refresh();


        }

        public void clearValues(byte[] squareArray)
        {
            for (int i = 0; i < 9; i++)
            {
                for (int j = 0; j < 9; j++)
                {
                    dataGridView1.Rows[i].Cells[j].Value = "";
                }
            }

        }

        private void dataGridView1_EditingControlShowing(object sender, DataGridViewEditingControlShowingEventArgs e)
        {
            dataGridView1.EditingControl.KeyPress -= EditingControl_KeyPress;
            dataGridView1.EditingControl.KeyPress += EditingControl_KeyPress;

        }
        private void EditingControl_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsControl(e.KeyChar))
            {
                Control editingControl = (Control)sender;
                if (editingControl.Text.Length > 0)
                {
                    e.Handled = true;
                }
                else
                {
                    char ch = e.KeyChar;

                    int charCode = (int) ch;
                    if (charCode < 0x31 || charCode > 0x39)
                    {
                        e.Handled = true;
                    }
                }
            }
        }

       private void dataGridView1_CellValidating(object sender,
       DataGridViewCellValidatingEventArgs e)
        {
            string text = e.FormattedValue.ToString(); 

            if (text.Length > 1)
            {
                
            }
            else if (text.Length > 0)
            {
                int charCode = (int)text[0];
                if (charCode < 0x31 || charCode > 0x39)
                {
                    dataGridView1.EditingControl.Text = "";
                }
            }

        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (inProgress)
            {
                return;
            }

            inProgress = true;

            for (int i = 0; i < 9; i++)
            {
                for (int j = 0; j < 9; j++)
                {         
                    if (dataGridView1.Rows[i].Cells[j].Value != null && dataGridView1.Rows[i].Cells[j].Value.ToString().Length > 0)
                        array[(i * 9) + j] = Convert.ToByte(dataGridView1.Rows[i].Cells[j].Value.ToString());
                    else
                        array[(i * 9) + j] = 0;
                }
            }
            size = Marshal.SizeOf(array[0]) * array.Length;
            pnt = Marshal.AllocHGlobal(size);

            try
            {
                // Copy the array to unmanaged memory.
                Marshal.Copy(array, 0, pnt, array.Length);
                solve(pnt, size);

                byte[] array2 = new byte[array.Length];
                Marshal.Copy(pnt, array2, 0, array.Length);
                setValues(array2);

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            finally
            {
                // Free the unmanaged memory.
                Marshal.FreeHGlobal(pnt);
                inProgress = false;
            }

            
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (inProgress)
            {
                return;
            }

            openFileDialog.ShowDialog();

        }

        private void openFileDialog_FileOk(object sender, CancelEventArgs e)
        {
            string filename = openFileDialog.FileName;
            bool validData = true;

            if (inProgress)
            {
                return;
            }

            clearValues(array);

            StreamReader sr = new StreamReader(filename);
            char[] buffer = new char[81];

            int readCount = sr.ReadBlock(buffer, 0, 81);

            sr.Close();

            if (readCount == 81)
            {
                array = Encoding.UTF8.GetBytes(buffer);

                for (int i = 0; i < 81; i++)
                {
                    if (array[i] < 0x30 || array[i] > 0x39)
                    {
                        validData = false;
                        break;
                    }

                    array[i] -= 48;
                }

                if (validData)
                {
                    setValues(array);
                }
            }
            else
            {
                validData = false;
                //TODO: Display error message
            }

            if (!validData)
            {
                MessageBox.Show("Invalid file");
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (inProgress)
            {
                return;
            }

            inProgress = true;

            ProgressCallback callback = new ProgressCallback(onCallback);

            for (int i = 0; i < 9; i++)
            {
                for (int j = 0; j < 9; j++)
                {
                    if (dataGridView1.Rows[i].Cells[j].Value != null && dataGridView1.Rows[i].Cells[j].Value.ToString().Length > 0)
                        array[(i * 9) + j] = Convert.ToByte(dataGridView1.Rows[i].Cells[j].Value.ToString());
                    else
                        array[(i * 9) + j] = 0;
                }
            }
            size = Marshal.SizeOf(array[0]) * array.Length;
            pnt = Marshal.AllocHGlobal(size);
            

            try
            {
                // Copy the array to unmanaged memory.
                Marshal.Copy(array, 0, pnt, array.Length);
                showSteps(pnt, size, callback);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            finally
            {
                // Free the unmanaged memory.
                Marshal.FreeHGlobal(pnt);
                inProgress = false;
            }

        }

        private void button4_Click(object sender, EventArgs e)
        {
            if (inProgress)
            {
                return;
            }

            clearValues(array);
        }
        
    }
}
