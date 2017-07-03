#!/usr/bin/env python

# Copyright (c) 2013 Freescale Semiconductor, Inc.
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

from __future__ import print_function
import sys
import os
import argparse
import datetime
import tempita
import xunitparser
import subprocess
from UserDict import UserDict
import xlsxwriter

## The Tempita template used to generate the HTML test report.
REPORT_TEMPLATE = """
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Test Report</title>
<style type="text/css">

#title {
    font-size: 170%;
    font-weight: bold;
    margin-bottom: 0px;
}

#subtitle {
    text-align: left;
}

#timestamp {
    font-size: 80%;
    margin-top: 15px;
}

#subhead {
    margin-top: 5px;
    margin-bottom: 20px;
}

table.report {
    width: 100%;
}

th.tablehdr {
    text-align: left;
    font-size: 120%;
    background-color: rgb(220,220,220);
    padding: 5px;
}

td.totalstype {
    font-size: 85%;
    font-weight: bold;
}

td.totalsvalue {
    font-size: 85%
}

td.classname {
    font-size: 120%;
    border-bottom: dashed 1px;
    border-color: rgb(120,120,120);
    text-align: left;
    padding: 20px 5px 0px 5px;
}

td.testresult {
    font-size: 85%;
    padding: 0px 10px 0px 5px;
}

.goodresult {
    color: rgb(0,255,0);
}

.skipresult {
    color: rgb(51,153,255);
}

.badresult {
    color: rgb(255,0,0);
    font-weight: bold;
}

tr.grouptotalsrow {
}

td.grouptotals {
    border-top: dashed 1px;
    border-color: rgb(150,150,150);
    font-size: 90%;
    padding: 0px 10px 0px 5px;
}

.grouptotalname {
    font-weight: bold;
}

</style>
</head>
<body>
<h2 id="title">{{title}}</h2>
<div id="subhead">
<div id="subtitle">{{subtitle}}</div>
<div id="timestamp">{{datetime.datetime.today().ctime()}}</div>
{{if revision}}<div id="revision">SHA-1 {{revision}}</div>{{endif}}
</div>
<table width="40%">
    <tr>
        <th class="tablehdr" colspan="2">Totals</th>
    </tr>
    <tr>
        <td class="totalstype">Cases</td>
        <td class="totalsvalue">{{totals.total}}</td>
    </tr>
    <tr>
        <td class="totalstype">Passes</td>
        <td class="totalsvalue">{{totals.passes}}</td>
    </tr>
    <tr>
        <td class="totalstype">Skips</td>
        <td class="totalsvalue">{{totals.skips}}</td>
    </tr>
    <tr>
        <td class="totalstype">Fails</td>
        <td class="totalsvalue">{{totals.fails}}</td>
    </tr>
    <tr>
        <td class="totalstype">Elapsed</td>
        <td class="totalsvalue">{{totals.elapsed}}</td>
    </tr>
</table>
<br/>
<table class="report">
    <tr>
        <th class="tablehdr">Test Case</th>
{{for columnName in columnNames}}
        <th class="tablehdr">{{columnName}}</th>
{{if showElapsedTime}}
        <th class="tablehdr">Time</th>
{{endif}}
{{endfor}}
    </tr>
    <tr>
        <td class="totalstype">Total Cases</td>
{{for columnName in columnNames}}
        <td class="totalsvalue">{{totalsByColumn[columnName].total}}</td>
{{endfor}}
    </tr>
    <tr>
        <td class="totalstype">Total Passes</td>
{{for columnName in columnNames}}
        <td class="totalsvalue">{{totalsByColumn[columnName].passes}}</td>
{{endfor}}
    </tr>
    <tr>
        <td class="totalstype">Total Skips</td>
{{for columnName in columnNames}}
        <td class="totalsvalue">{{totalsByColumn[columnName].skips}}</td>
{{endfor}}
    </tr>
    <tr>
        <td class="totalstype">Total Fails</td>
{{for columnName in columnNames}}
        <td class="totalsvalue">{{totalsByColumn[columnName].fails}}</td>
{{endfor}}
    </tr>
    <tr>
        <td class="totalstype">Total Elapsed</td>
{{for columnName in columnNames}}
        <td class="totalsvalue">{{totalsByColumn[columnName].elapsed}}</td>
{{endfor}}
    </tr>
    <tr><td colspan="{{columnCount + 1}}"><td></tr>
{{for groupName in groupNames}}
{{py:
groupColumns = groups[groupName]
}}
    <tr class="grouptitlerow">
        <td colspan="{{columnCount + 1}}" class="classname">{{groupName}}</td>
    </tr>
{{for caseId in caseIdsByGroup[groupName]}}
    <tr class="casedetailrow">
        <td class="testresult"><span class="methodname">{{caseNamesById[caseId]}}</span></td>
{{for columnName in columnNames}}
{{py:
try:
    group = groups[groupName][columnName]
    case = group.caseIdMap[caseId]
except KeyError:
    case = None
}}
{{if case is None}}
        <td class="testresult">n/a</td>
{{else}}
        <td class="testresult"><span class="{{if case.success}}goodresult{{elif case.skipped}}skipresult{{else}}badresult{{endif}}">{{if case.success}}passed{{elif case.skipped}}skipped{{else}}failed{{endif}}</span></td>
{{endif}}{{py:##if case is none}}
{{endfor}}{{py:##end: for columnName}}
    </tr>
{{endfor}}{{py:##end: for caseId}}
    <tr class="grouptotalsrow">
        <td class="grouptotals"><span class="grouptotalname">Totals (pass/skip/fail)</span></td>
{{for columnName in columnNames}}
{{py:
ok = True
try:
    group = groups[groupName][columnName]
except KeyError:
    ok = False
}}
{{if ok}}
        <td class="grouptotals">{{group.totals.passes}}/{{group.totals.skips}}/{{group.totals.fails}}</td>
{{else}}
        <td class="grouptotals">n/a</td>
{{endif}}{{py:##ok}}
{{endfor}}{{py:##end: for columnName}}
    </tr>
{{endfor}}{{py:##end: for groupName}}
</table>
</body>
</html>
"""

# {{py:##if (not showOnlyFailures) or (showOnlyFailures and not case.good)}}

# {{if showElapsedTime}}
#         <td class="testresult"><span class="testtime">{{case.time}}</span></td>
# {{endif}}

# {{if showElapsedTime}}
#         <td class="grouptotals">{{group.totalTime}}</td>
# {{endif}}

##
# @brief
class GitRevision:
    def __init__(self):
        self.revision = ""
        self.headline = ""

    def read_revision(self):
        try:
            out = subprocess.check_output(['git', 'log', '-n', '1', '--pretty=oneline'], stderr=None)
        except subprocess.CalledProcessError:
            # git failed, but just ignore it
            pass
        else:
            self.revision = out[:40]
            self.headline = out[41:]

##
# @brief Base class for report generators.
class TestReportGenerator(object):
    def __init__(self, info, outputFile):
        ## Test report processor object.
        self.testInfo = info

        ## Output file name.
        self.outputFile = outputFile

        # Set default values for report options.
        self.title = "Test Report"
        self.subtitle = ""
        self.showOnlyFailures = False
        self.showSkipped = False
        self.showElapsedTime = False
        self.revision = ""
        self.headline = ""

#         gitReader = GitRevision()
#         gitReader.read_revision()
#         self.revision = gitReader.revision
#         self.headline = gitReader.headline

    def generate(self):
        pass

##
# @brief Creates the output test report given a set of options.
class HTMLTestReportGenerator(TestReportGenerator):
    def __init__(self, info, outputFile):
        super(HTMLTestReportGenerator, self).__init__(info, outputFile)

        # Load Tempita template.
        self._namespace = tempita.bunch()
        self._template = tempita.HTMLTemplate(content=REPORT_TEMPLATE, name='Test Report', namespace=self._namespace)

    def generate(self):
        import datetime

        # Update the template namespace.
        self._namespace.update({
            'title' : self.title,
            'subtitle' : self.subtitle,
            'datetime' : datetime,
            'info' : self.testInfo,
            'suites' : self.testInfo.suites,
            'columnCount' : self.testInfo.columnCount,
            'columnNames' : self.testInfo.columnNames,
            'cases' : self.testInfo.cases,
            'classNames' : self.testInfo.classNames,
            'casesByClassname' : self.testInfo.casesByClassName,
            'caseNamesByGroup' : self.testInfo.caseNamesByGroup,
            'caseIdsByGroup' : self.testInfo.caseIdsByGroup,
            'caseNamesById' : self.testInfo.caseNamesById,
            'groups' : self.testInfo.classGroups,
            'groupNames' : self.testInfo.groupNames,
            'showOnlyFailures' : self.showOnlyFailures,
            'showElapsedTime' : self.showElapsedTime,
            'totals' : self.testInfo.totals,
            'totalsByColumn' : self.testInfo.totalsByColumn,
            'revision' : self.revision,
            'headline' : self.headline
            })

        # Generate the output.
        with open(self.outputFile, 'w') as f:
            f.write(self._template.substitute())

##
# @brief Creates the output test report given a set of options.
class ExcelTestReportGenerator(TestReportGenerator):
    def __init__(self, info, outputFile):
        super(ExcelTestReportGenerator, self).__init__(info, outputFile)

        self.workbook = xlsxwriter.Workbook(outputFile)
        self.worksheet = self.workbook.add_worksheet('Report')

        self.columnHeadStyle = self.workbook.add_format({'bold' : True, 'font_size' : 18, 'bg_color' : '#d0d0d0'})


    def generate(self):
        # Write title, subtitle, timestamp.
        self.worksheet.write('A1', self.title)
        self.worksheet.write('A2', self.subtitle)
        self.worksheet.write('A4', datetime.datetime.today().ctime())

        row = 5

        # Write report column heads.
        self.worksheet.write(row, 0, "Test Case", self.columnHeadStyle)
        self.worksheet.set_column(0, 0, 40.0)

        for i, name in enumerate(self.testInfo.columnNames):
            self.worksheet.write(row, 1 + i, name, self.columnHeadStyle)
            self.worksheet.set_column(1 + i, 1 + i, 12.0)
        row += 1

        for groupName in self.testInfo.groupNames:
            groupColumns = self.testInfo.classGroups[groupName]

            # Group header
            self.worksheet.write(row, 0, groupName)
            row += 1

            # Test cases in group
            for caseId in self.testInfo.caseIdsByGroup[groupName]:
                # Test case name
                self.worksheet.write(row, 0, self.testInfo.caseNamesById[caseId])

                # Result columns for test case
                col = 1
                for columnName in self.testInfo.columnNames:
                    try:
                        group = self.testInfo.classGroups[groupName][columnName]
                        case = group.caseIdMap[caseId]
                    except KeyError:
                        case = None

                    if case is None:
                        self.worksheet.write(row, col, "n/a")
                    else:
                        if case.success:
                            text = "passed"
                        elif case.skipped:
                            text = "skipped"
                        else:
                            text = "failed"
                        self.worksheet.write(row, col, text)

                    col += 1
                row += 1

            # Totals row for group
            self.worksheet.write(row, 0, "Totals (pass/skip/fail)")

            # Totals columns for group
            col = 1
            for columnName in self.testInfo.columnNames:
                ok = True
                try:
                    group = self.testInfo.classGroups[groupName][columnName]
                except KeyError:
                    ok = False

                if ok:
                    totalsText = "%d/%d/%d" % (group.totals.passes, group.totals.skips, group.totals.fails)
                    self.worksheet.write(row, col, totalsText)
                else:
                    self.worksheet.write(row, col, "n/a")
                col += 1
            row += 2

        self.workbook.close()


##
# @brief
class TestTotals(object):
    def __init__(self):
        self.passes = 0
        self.skips = 0
        self.fails = 0
        self.total = 0
        self.elapsed = datetime.timedelta()

    def update(self, c):
        self.total += 1
        if c.success:
            self.passes += 1
        elif c.skipped:
            self.skips += 1
        else:
            self.fails += 1
        self.elapsed += c.time

##
# @brief
class TestCaseGroup(object):
    def __init__(self, name):
        self.name = name
        self.cases = []
        self.caseIdMap = {}
        self.totals = TestTotals()

    def add_case(self, case):
        self.cases.append(case)
        self.caseIdMap[case.id()] = case
        self.totals.total += 1

    def finalize(self):
        for c in self.cases:
            self.totals.update(c)

        self.cases.sort(key=lambda c:c.methodname)

##
# @brief Consumes and collates test case results.
#
# cases = { 'col1' : [test1, test2, ...], 'col2' : [test1, test2, ...] }
#
# caseNamesById = { 'id1' : 'case1', 'id2' : 'case2', ... }
#
# casesByClassName = { 'classA' : { 'col1' : [test1, test2, ...], 'col2' : [test1, test2, ...] },
#                      'classB' : { 'col1' : [test1, test2, ...], 'col2' : [test1, test2, ...] } }
#
# classGroups = { 'classA' : { 'col1' : testGroup1, 'col2' : testGroup2 },
#                 'classB' : { 'col1' : testGroup1, 'col2' : testGroup2 } }
#
# caseNamesByGroup = { 'classA' : set( 'case1', 'case2', ...), 'classB' : set( 'case3', ... ) }
#
# caseIdsByGroup = { 'classA' : set( 'id1', 'id2', ...), 'classB' : set( 'id3', ... ) }
#
#
class TestReportProcessor(object):
    def __init__(self):
        self.suites = {}
        self.cases = {}
        self.caseNamesById = {}
        self.classNames = set()
        self.caseNames = set()
        self.casesByClassName = {}
        self.classGroups = {}
        self.groupNames = set()
        self.columnNames = []
        self.columnCount = 0
        self.caseNamesByGroup = {}
        self.caseIdsByGroup = {}
        self.totals = TestTotals()
        self.totalsByColumn = {}

        self._columnSuites = {}
        self._currentColumn = None

    ##
    # @brief Read and process a JUnit-style XML test report file.
    def add_report_file(self, reportFile):
        suite, results = xunitparser.parse(reportFile)
        self._columnSuites[self._currentColumn].append(suite)
        #self.add_suite(suite)

    ##
    # @brief Start a new column of results.
    def start_column(self, name):
        self.columnNames.append(name)
        self._currentColumn = name
        self._columnSuites[name] = []
        self.columnCount += 1
        self.totalsByColumn[name] = TestTotals()

    ##
    # @brief Process the test cases in the provided suite.
    def add_suite(self, suite, col=''):
        if self.suites.has_key(col):
            self.suites[col].append(suite)
        else:
            self.suites[col] = [suite]

        for c in suite._tests:
            self.add_case(c, col)

    def add_case(self, c, col=''):
        # Update totals.
        self.totals.update(c)
        self.totalsByColumn[col].update(c)

        if self.cases.has_key(c.id()):
            self.cases[c.id()][col] = c
        else:
            self.cases[c.id()] = { col : c }
        self.classNames.add(c.classname)
        self.caseNames.add(c.id())
        self.caseNamesById[c.id()] = c.methodname

        if self.casesByClassName.has_key(c.classname):
            if self.casesByClassName[c.classname].has_key(col):
                self.casesByClassName[c.classname][col].append(c)
            else:
                self.casesByClassName[c.classname][col] = [c]
        else:
            self.casesByClassName[c.classname] = { col : [c] }

        if self.classGroups.has_key(c.classname):
            if self.classGroups[c.classname].has_key(col):
                group = self.classGroups[c.classname][col]
            else:
                group = TestCaseGroup(c.classname)
                self.classGroups[c.classname][col] = group
        else:
            group = TestCaseGroup(c.classname)
            self.classGroups[c.classname] = { col : group }

        group.add_case(c)

        if self.caseNamesByGroup.has_key(c.classname):
            self.caseNamesByGroup[c.classname].add(c.methodname)
        else:
            self.caseNamesByGroup[c.classname] = set([c.methodname])

        if self.caseIdsByGroup.has_key(c.classname):
            self.caseIdsByGroup[c.classname].add(c.id())
        else:
            self.caseIdsByGroup[c.classname] = set([c.id()])



    def finalize(self):
        uniqueTestNames = set()

        for column, suites in self._columnSuites.items():
            #print(name, " = ", suites)

            for suite in suites:
                self.add_suite(suite, column)

#         print(uniqueTestNames)

        groupNamesSet = set()
        for className, groupDict in self.classGroups.items():
            for colName, group in groupDict.items():
                group.finalize()
                groupNamesSet.add(className)

        self.groupNames = list(groupNamesSet)
        self.groupNames.sort()

#         print(self.cases)
#         sys.exit()

    def print_results(self):
        for c in self.cases:
            print(c.classname, c.methodname, "-", c.result)


#       process_test_report.py -c I2C -i test_results_i2c.xml -c SPI -i test_results_spi.xml
#
#       process_test_report.py col=I2C test_results_i2c.xml col=SPI test_results_spi.xml

##
# @brief Command-line interface to the tool.
class TestReportProcessorTool(object):
    def __init__(self):
        pass

    def _read_options(self, opts):
        # Build arg parser.
        parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter)

        parser.add_argument("-t", "--title", help="Set the output report title.")
        parser.add_argument("-b", "--subtitle", help="Set the output report subtitle.")
        parser.add_argument("-f", "--failures", action="store_true", default=False, help="Show only failed tests.")
        parser.add_argument("-s", "--skipped", action="store_true", default=False, help="Show skipped tests.")
        parser.add_argument("--times", action="store_true", default=False, help="Show elapsed time column.")
        parser.add_argument("-o", "--output", required=True, metavar="PATH", help="Specify the output file.")
        parser.add_argument("--html", action="store_true", default=True, help="Output in HTML format (default).")
        parser.add_argument("--excel", action="store_true", default=False, help="Output in Excel xlsx format.")
        parser.add_argument("reports", nargs='*', help="Test report XML file.")

        if opts: 
            return parser.parse_args(opts)
        else:
            return parser.parse_args()

    def run(self, opts=None):
        # Read command line arguments.
        args = self._read_options(opts)

        if not len(args.reports):
            print("Error: test report file required")
            return 1

        if args.output is None:
            print("Error: failed to open output file")
            return 1

        # Create test report processor.
        self._processor = TestReportProcessor()
        self._add_reports(args.reports)
        self._processor.finalize()

        # Create report generator and produce the report.
        if args.excel:
           genClass = ExcelTestReportGenerator
        elif args.html:
            genClass = HTMLTestReportGenerator
        gen = genClass(self._processor, args.output)

        gen.showOnlyFailures = args.failures
        gen.showSkipped = args.skipped
        gen.showElapsedTime = args.times
        if args.title is not None:
            gen.title = args.title
        if args.subtitle is not None:
            gen.subtitle = args.subtitle

        gen.generate()

        return 0

    def _add_reports(self, reports):
        didStartColumn = False
        for r in reports:
            if r.startswith('%col='):
                name = r.lstrip('%col=')
                self._processor.start_column(name)
                didStartColumn = True
            else:
                if not didStartColumn:
                    self._processor.start_column('Result')
                    didStartColumn = True
                self._processor.add_report_file(open(r, 'r'))


# Create the main class and run it.
if __name__ == "__main__":
    exit(TestReportProcessorTool().run())

