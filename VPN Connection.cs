using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.IO;


//copyright 2019 Tomasz Krokosz

namespace RaspberryClientForms
{
    public partial class TemperatureUserControl : UserControl
    {
        Bitmap img;
        Bitmap bar;

        public TemperatureUserControl()
        {
            InitializeComponent();
            comboBox1.Items.Add("Termometr 1 (sala 1)");
            comboBox1.Items.Add("Termometr 2 (sala 2)");

        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!String.IsNullOrEmpty(comboBox1.Text))
                panelResult.Visible = true;
            else
                panelResult.Visible = false;

            textBox1.Text = "";
            label4.Text = "";
            label7.Text = "";
            label6.Text = "";

            label6.Text = "";
            string device_no = "";
            string token = "";
            string type = "";
            string operation = "";
            if (comboBox1.Text == "Termometr 1 (sala 1)")
            {
                device_no = "0db82470-5a50-11ef-aa15-a127638e3a77";
                token = "VD98uWM2IZVlKiPHA4fS";
                type = "temperature";
                operation = "R";
            }

            if (comboBox1.Text == "Termometr 2 (sala 2)")
            {
                device_no = "7aeecf20-5a4c-11ef-8d66-9bbb0351be2a";
                token = "xsLOnOFrrLC1fOuJm8zN";
                type = "temperature";
                operation = "R";
            }

            string ip_address = "";

            string response = Connect(ip_address, (device_no + "____" + token + "____" + type + "____" + operation + "____" + "empty").ToString(), "R");

            string[] resp_after_split = response.Split('_');

            var a = resp_after_split;

            Regex regex = new Regex("value\":\"" + @"\d+\""");
            Match match = regex.Match(resp_after_split[0]);

            string first = "";
            string second = "";


            if (match.Success)
            {
                Regex regex1 = new Regex(@"\d+");
                Match match1 = regex1.Match(match.Value);
                if (match1.Success)
                {
                    first = match1.Value;
                }

            }
            else
                label4.Text = response;

            if (resp_after_split.Count() >= 2)
            {
                Match match_sec = regex.Match(resp_after_split[2]);
                if (match_sec.Success)
                {
                    Regex regex1 = new Regex(@"\d+");
                    Match match_sec_1 = regex1.Match(match_sec.Value);
                    if (match_sec_1.Success)
                    {
                        second = match_sec_1.Value;
                    }

                }
                else
                    label4.Text = response;
            }
            

            label7.Text = "Ustawiona temperatura: " + first + ", maksymalna temperatura: " + second;
            label4.Text = first;
            if (!String.IsNullOrEmpty(second) && !String.IsNullOrEmpty(first))
            {
                if (Convert.ToDouble(second) < Convert.ToDouble(first))
                {
                    label6.Text = "PRZEKROCZONO DOPUSZCZALNĄ TEMPERATURĘ";
                }
            }
        }
    

        private void panelResult_Paint(object sender, PaintEventArgs e)
        {

        }

       

        private void button1_Click(object sender, EventArgs e)
        {
            

            label6.Text = "";
            string device_no = "";
            string token = "";
            string type = "";
            string operation = "";


            if (comboBox1.Text == "Termometr 1 (sala 1)")
            {
                device_no = "0db82470-5a50-11ef-aa15-a127638e3a77";
                token = "VD98uWM2IZVlKiPHA4fS";
                type = "temperature";
                operation = "U";
            }
                
            if (comboBox1.Text == "Termometr 2 (sala 2)")
            {
                device_no = "eda0d2d0-e814-11e8-b45e-f1cc3c52f3a0";
                token = "xsLOnOFrrLC1fOuJm8zN";
                type = "temperature";
                operation = "U";
            }

            int new_value = 0;

            int res;

            if (int.TryParse(textBox1.Text, out res))
            {
                new_value = Convert.ToInt32(textBox1.Text);
            }

            DateTime StartTime = DateTime.Now;
            DateTime EndTime = DateTime.Now;

            string ip_address = "";

            for (var i = 1; i <= 10000; i++)
            {
                StartTime = DateTime.Now;

                new_value = (i % 23);

                

                string response = Connect(ip_address, (device_no + "____" + token + "____" + type + "____" + operation + "____" + textBox1.Text).ToString(), operation);

                Regex regex = new Regex("value\":\"" + @"\d+\""");
                Match match = regex.Match(response);
                if (match.Success)
                {
                    Regex regex1 = new Regex(@"\d+");
                    Match match1 = regex1.Match(match.Value);
                    if (match1.Success)
                    {
                        label7.Text = "Ustawiona temperatura: " + textBox1.Text.ToString() + ", maksymalna temperatura: " + match1.Value.ToString();
                        label4.Text = textBox1.Text.ToString();
                        if (Convert.ToDouble(match1.Value) < Convert.ToDouble(textBox1.Text))
                        {
                            label6.Text = "PRZEKROCZONO DOPUSZCZALNĄ TEMPERATURĘ";
                        }
                    }

                }
                else
                    label7.Text = response.Replace(device_no, "").Replace(token, "").Replace(type, "").Replace(operation, "").Replace("____", "");


                EndTime = DateTime.Now;

                TimeSpan ts = EndTime - StartTime;

                string path = "time_results.txt";
                if (!File.Exists(path))
                {
                    // Create a file to write to.
                    using (StreamWriter sw = File.CreateText(path))
                    {
                        sw.WriteLine(Math.Round(((double)ts.Milliseconds / 1000), 8).ToString()); // Write text to .txt file
                    }
                }
                else // If file already exists
                {
                    // File.WriteAllText("FILENAME.txt", String.Empty); // Clear file
                    using (StreamWriter sw = File.AppendText(path))
                    {
                        sw.WriteLine(Math.Round(((double)ts.Milliseconds / 1000), 8).ToString()); // Write text to .txt file
                    }
                }
            }

        }

        static string Connect(String server, String message, string operation)
        {
            try
            {
                // Create a TcpClient.
                // Note, for this client to work you need to have a TcpServer 
                // connected to the same address as specified by the server, port
                // combination.
                Int32 port = 0; // numer portu dla połączenia VPN

                TcpClient client = new TcpClient(server, port);

                // Translate the passed message into ASCII and store it as a Byte array.
                Byte[] data = System.Text.Encoding.ASCII.GetBytes(message);

                // Get a client stream for reading and writing.
                //  Stream stream = client.GetStream();

                NetworkStream stream = client.GetStream();

                // Send the message to the connected TcpServer. 
                stream.Write(data, 0, data.Length);

                Console.WriteLine("Wysłano: {0}", message);

                // Receive the TcpServer.response.

                // Buffer to store the response bytes.
                data = new Byte[1024];

                // String to store the response ASCII representation.
                String responseData = String.Empty;

                // Read the first batch of the TcpServer response bytes.
                Int32 bytes = stream.Read(data, 0, data.Length);
                responseData = System.Text.Encoding.ASCII.GetString(data, 0, bytes);
                Console.WriteLine("Otrzymano: {0}", responseData);
                
                // Close everything.
                stream.Close();
                client.Close();

                return responseData;
            }
            catch (ArgumentNullException e)
            {
                Console.WriteLine("ArgumentNullException: {0}", e);
            }
            catch (SocketException e)
            {
                Console.WriteLine("SocketException: {0}", e);
            }

            Console.WriteLine("\n Enter - zamknij");
            Console.Read();
            return "";
        }

        private void button2_Click(object sender, EventArgs e)
        {
            label6.Text = "";
            
            string device_no = "";
            string token = "";
            string type = "";
            string operation = "";
            if (comboBox1.Text == "Termometr 1 (sala 1)")
            {
                device_no = "0db82470-5a50-11ef-aa15-a127638e3a77";
                token = "VD98uWM2IZVlKiPHA4fS";
                type = "temperature";
                operation = "R";
            }

            if (comboBox1.Text == "Termometr 2 (sala 2)")
            {
                device_no = "eda0d2d0-e814-11e8-b45e-f1cc3c52f3a0";
                token = "xsLOnOFrrLC1fOuJm8zN";
                type = "temperature";
                operation = "R";
            }

            DateTime StartTime = DateTime.Now;
            DateTime EndTime = DateTime.Now;
            string ip_address = "";

            for (int i = 1; i <= 10000; i++)
            {
                StartTime = DateTime.Now;

                string response = Connect(ip_address, (device_no + "____" + token + "____" + type + "____" + operation + "____" + "empty").ToString(), "R");

                string[] resp_after_split = response.Split('_');

                var a = resp_after_split;

             //   MessageBox.Show(response);

                Regex regex = new Regex("value\":\"" + @"\d+\""");
                Match match = regex.Match(resp_after_split[0]);

                string first = "";
                string second = "";


                if (match.Success)
                {
                    Regex regex1 = new Regex(@"\d+");
                    Match match1 = regex1.Match(match.Value);
                    if (match1.Success)
                    {
                        first = match1.Value;
                        label4.Text = first;
                    }

                }
                else
                    label4.Text = response;

                if (resp_after_split.Count() >= 2)
                {
                    Match match_sec = regex.Match(resp_after_split[2]);
                    if (match_sec.Success)
                    {
                        Regex regex1 = new Regex(@"\d+");
                        Match match_sec_1 = regex1.Match(match_sec.Value);
                        if (match_sec_1.Success)
                        {
                            second = match_sec_1.Value;
                        }

                    }
                    else
                        label4.Text = response;
                }

                label7.Text = "Ustawiona temperatura: " + first + ", maksymalna temperatura: " + second;

                label4.Text = first;

                if (!String.IsNullOrEmpty(second) && !String.IsNullOrEmpty(first))
                {
                    if (Convert.ToDouble(second) < Convert.ToDouble(first))
                    {
                        label6.Text = "PRZEKROCZONO DOPUSZCZALNĄ TEMPERATURĘ";
                    }
                }

                EndTime = DateTime.Now;

                TimeSpan ts = EndTime - StartTime;

                string path = "read_time_results.txt";
                if (!File.Exists(path))
                {
                    // Create a file to write to.
                    using (StreamWriter sw = File.CreateText(path))
                    {
                        sw.WriteLine("First line - start file");
                    }
                }
                else // If file already exists
                {
                    // File.WriteAllText("FILENAME.txt", String.Empty); // Clear file
                    using (StreamWriter sw = File.AppendText(path))
                    {
                        sw.WriteLine(Math.Round(((double)ts.Milliseconds / 1000), 8).ToString()); // Write text to .txt file
                    }
                }
            }
            
            
        }
        

        private void TemperatureUserControl_Load(object sender, EventArgs e)
        {
            
       //     img = RaspberryClientForms.I
        //    bar = Properties.Resources.bar1;
            //make Color.White transparent in bar image
        //    bar.MakeTransparent(Color.White);
            //trigger NumericUpDowns.ValueChanged
            //   NumericUpDown1.Value += 1;
            //   NumericUpDown1.Value -= 1;
        }

        private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar) && (e.KeyChar != '.'))
            {
                e.Handled = true;
            }

            // only allow one decimal point
            if ((e.KeyChar == '.') && ((sender as TextBox).Text.IndexOf('.') > -1))
            {
                e.Handled = true;
            }
        }
    }
}
