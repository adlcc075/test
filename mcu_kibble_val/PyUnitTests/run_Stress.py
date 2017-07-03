#! /usr/bin/env python

# Copyright (c) 2014 Freescale Semiconductor, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# o Redistributions of source code must retain the above copyright notice, this list
#   of conditions and the following disclaimer.
#
# o Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# o Neither the name of Freescale Semiconductor, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
import sys
import os
import glob
import re
import subprocess
import time
import datetime
import inspect
import ConfigParser

import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.image import MIMEImage
from email.mime.base import MIMEBase
from email import encoders
from email import Utils
from email.header import Header

sys.path.append(os.path.abspath("../testReport"))
import process_test_report

#when run stress test skip running test_blltloader_for_all_ide. 
os.environ['BOOTLOADER_TEST_FOR_ALL_IDE'] = '0'



class TestLoop:
    def __init__(self):
        # Get current path
        this_file = inspect.getfile(inspect.currentframe())
        current_path = os.path.abspath(os.path.dirname(this_file)).replace('\\', '/')

        # Get test config info from external file
        test_cfg_file = os.path.join(current_path, '..', 'targets', 'test_config.ini')
        if not os.path.isfile(test_cfg_file):
            raise RuntimeError("No such configuration file: %s" %test_cfg_file)

        config = ConfigParser.SafeConfigParser()
        config.read(test_cfg_file)
        self.targetName                     =   config.get('config', 'cpu')
        self.projectName                   =   config.get('config', 'board')
        self.buildVersion                   =   config.get('config', 'build')
        self.kibble_serial_port             =   config.get('config', 'serial_port')
        self.kibble_assistant_serial_port   =   config.get('config', 'assistant_serial_port')
        self.jlinkUsbId            =   config.get('config', 'jlinkUsbId')
        self.kibble_project_image           =   config.get('config', 'project_image')

        # Get the peripheral info from peripheral_config.ini
        peripheral_info_cfg_file = os.path.join(current_path, '..', 'targets', self.targetName, 'peripheral_config.ini')
        if not os.path.isfile(peripheral_info_cfg_file):
            raise RuntimeError("No such configuration file: %s" %peripheral_info_cfg_file)

        config = ConfigParser.SafeConfigParser()
        config.read(peripheral_info_cfg_file)
        self.kibble_uart_support = config.getboolean('supported_peripherals', 'uart_support')
        self.kibble_i2c_support  = config.getboolean('supported_peripherals', 'i2c_support')
        self.kibble_spi_support  = config.getboolean('supported_peripherals', 'spi_support')
        self.kibble_can_support  = config.getboolean('supported_peripherals', 'can_support')
        self.kibble_usb_support  = config.getboolean('supported_peripherals', 'usb_support')

        self.testConfigs = []
        if self.kibble_uart_support:
            uart_speed = config.get('speed', 'uart_speed')
            uart_info  = ('uart', uart_speed, self.kibble_serial_port, 'True')
            self.testConfigs.append(uart_info)
        else:
            pass

        if self.kibble_i2c_support:
            i2c_speed = config.get('speed', 'i2c_speed')
            i2c_info  = ('i2c', i2c_speed, self.kibble_assistant_serial_port, 'True')
            self.testConfigs.append(i2c_info)
        else:
            pass

        if self.kibble_spi_support:
            spi_speed = config.get('speed', 'spi_speed')
            spi_info  = ('spi', spi_speed, self.kibble_assistant_serial_port, 'True')
            self.testConfigs.append(spi_info)
        else:
            pass

        if self.kibble_can_support:
            can_speed = config.get('speed', 'can_speed')
            can_info  = ('can', can_speed, self.kibble_assistant_serial_port, 'True')
            self.testConfigs.append(can_info)
        else:
            pass

        if self.kibble_usb_support:
            usb_info  = ('usb', '', '', 'True')
            self.testConfigs.append(usb_info)
        else:
            pass

    def run(self):
        ts = time.time()
        dirname = 'reports-' + datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d-%H-%M-%S')
        count = 0
        # Generate a directory to store the test reports.
        report_dirname = os.path.join(os.path.abspath('../testReport/'), dirname)
        os.makedirs(report_dirname)

        # Make sure stress test runs under default clock frequency.
        os.environ['CLOCK_FLAG'] = str(0xFF)
        os.environ['CLOCK_DIV'] = str(0xFF)

        # Set the run cycles for very peripheral in self.testConfigs.
        # For example, ROUND = 5 means all the peripherals in self.testConfigs will run 5 times tests.
        ROUND = 1
        total_cycles = ROUND * len(self.testConfigs)
        i = 0
        while i < total_cycles:
            index = i % len(self.testConfigs)

            # Set the test peripheral (UART, I2C, SPI, USB)
            os.environ['KIBBLE_PERIPHERAL'] = self.testConfigs[index][0]
            os.environ['KIBBLE_SPEED'] = self.testConfigs[index][1]
            # Set the serial port for the chosen peripheral
            os.environ['KIBBLE_PORT'] = self.testConfigs[index][2]
            # Set the initial ping of a serial targe
            os.environ['KIBBLE_USE_PING'] = self.testConfigs[index][3]

            os.environ['KIBBLE_TARGET_NAME']                = self.targetName
            os.environ['KIBBLE_PROJECT_NAME']              = self.projectName
            os.environ['KIBBLE_BUILD']              = self.buildVersion
            os.environ['JLINK_USB_ID']              = self.jlinkUsbId
            os.environ['KIBBLE_TARGET_UART_PORT']   = self.kibble_serial_port
            os.environ['KIBBLE_BUSPAL_UART_PORT']   = self.kibble_assistant_serial_port


            # Generate the test report file (.xml) and store it in report_dirname directory.
            reportName = os.path.join(    report_dirname,
                                          ('test_report' + '-' +
                                           str(i) + '-' +
                                           self.testConfigs[index][0] + '-' +
                                           str(self.testConfigs[index][1]) + '-' +
                                           str(self.testConfigs[index][3]) +
                                           '.xml'
                                           )
                                      )
            # Call all the unit test cases (The test_xxx.py files)
            # Should add "PytestPath" as a new system variable, the value is the path of the pytest.py in the local computer.

            subprocess.call([sys.executable, os.path.expandvars('%PytestPath%/pytest.py'), '--junitxml=' + reportName])

            # subprocess.call([sys.executable, os.path.expandvars('%PytestPath%/pytest.py'), '-v' , '-s','--junitxml=' + reportName])
            i = i + 1

        return report_dirname

    def generateTestReport(self, report_dirname, title, subtitle):
        peripherals = []
        peripheralReports = []
        for i in range(0, len(self.testConfigs)):
            peripherals.append(self.testConfigs[i][0])
            peripheralReports.append({})

        # Change the working directory to the specific path.
        os.chdir(report_dirname)

        # Get the peripheral run times from the .xml file name.
        # For example, if file name is 'test_report-5-i2c-False.xml' we get 5.
        for file in glob.glob('test_report*.xml'):
            m = re.search('\d+', file)
            testRun = int(m.group())
            peripheralReports[testRun % len(peripherals)][testRun] = file

        args = ['--title='+title, '--subtitle='+subtitle, '-o', title+'_'+subtitle+'_'+'test_reports.html', '--']
        for peripheralReport in peripheralReports:
            for runKey in sorted(peripheralReport.iterkeys()):
                args.append('%col=' + peripherals[runKey % len(peripherals)] + '\nRun' + str(runKey / len(peripherals) + 1))
                args.append(peripheralReport[runKey])

        process_test_report.TestReportProcessorTool().run(args)
        # return the file name of the test report
        report_file_name = args[3]
        return report_file_name


class SendEmail():
    def __init__(self, mail_to_list, mail_cc_list):
        self.mail_host = 'remotesmtp.freescale.net'
        self.mail_from = 'XiaoPeng' + '@' + 'freescale.com'
        self.mail_to = mail_to_list
        self.mail_cc = mail_cc_list

    def send_email(self, subject, content, attachment):
        # Create a root container
        main_msg = MIMEMultipart('alternative')
        # Create a text container to show the contents of the email
        text_msg = MIMEText(content, 'plain', 'utf-8')

        # Attach the contents
        main_msg.attach(text_msg)
        contype = 'application/octet-stream'
        maintype, subtype = contype.split('/', 1)
        # Create attachment
        att = open(attachment, 'rb')
        att_msg = MIMEBase(maintype, subtype)
        # Read the contents of the attachment and format it
        att_msg.set_payload(att.read())
        att.close()
        encoders.encode_base64(att_msg)
        # Add the header for attachment
        att_msg.add_header('Content-Disposition', 'attachment', filename = attachment.split('\\')[-1])
        main_msg.attach(att_msg)
        # Set the attibutes for the root container
        main_msg['Subject'] = subject
        main_msg['From'] = self.mail_from
        main_msg['To'] = ','.join(self.mail_to)
        main_msg['Cc'] = ','.join(self.mail_cc)
        main_msg['Date'] = Utils.formatdate()

        server = smtplib.SMTP()
        server.connect(self.mail_host)
        server.sendmail(self.mail_from, self.mail_to+self.mail_cc, main_msg.as_string())
        server.quit()



## ==============================================================================================
# ==============================================================================================
# if __name__ == "__main__":
# print hello
stress_test = TestLoop()
report_dirname = stress_test.run()
filename = stress_test.generateTestReport(report_dirname, stress_test.targetName, stress_test.projectName)
# Get the full path of the test report file
test_report = os.path.join(report_dirname, filename)

# Send the test report via email
email = SendEmail(mail_to_list = ['fangqiu.wang@nxp.com',
                                  'fanny.fan@nxp.com',
                                  'yong.xiong@nxp.com',
                                  'zhenzhen.liu@nxp.com',
                                  'lei.hu@nxp.com',
                                  'xiangyi.cai@nxp.com',
                                  'zengxing.chen@nxp.com'
                                  ],
                  mail_cc_list = []) # kibtest@freescale.com
subject = 'bootloader test result on ' + stress_test.targetName.upper()
content = 'Hi All,\n\n' + \
          'This email is automatically sent by Kibble Automation System.\n' + \
          'Please DO NOT reply this email. Thanks.\n\n' + \
          'Regards,\n' + \
          'Kibble Automation Test'

exit(email.send_email(subject = subject, content = content, attachment = test_report))
