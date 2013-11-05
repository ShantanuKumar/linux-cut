// Implements reduced functionality version of linux command line tool cut
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <cstdlib>
using namespace std;

#define END -2        // Dummy parameter to indicate the end of line in the file when we don't know exact location.
#define RES_OK 0      // Indicates arguments/inputs provided are correct
#define BAD_INPUT -3 // Indicates arguments/inputs provided are incorrect

/* Checks whether a string is a number or not */
bool is_number (const string& s)
{
    string::const_iterator it = s.begin();
    while ((it != s.end()) && isdigit(*it)) ++it;
    return (!s.empty() && it == s.end());
}

/* Displaying the bytes based on the parameter bytepos */
void readBytes(const string& fileName, const vector<int>& bytePos)
{
    ifstream fileToRead (fileName.c_str());
    string line;
    size_t length = bytePos.size();
    if (fileToRead.is_open())
    {
        while(getline(fileToRead,line))
        {
            if (length == 1)
                cout << line[bytePos[0]];

            for(unsigned i = 0; i < length -1 ; ++i)
            {
                if (bytePos[i+1] == END)
                {
                    cout << line.substr(bytePos[i]);
                    break;
                }
                else
                {
                    cout << line[bytePos[i]];
                    if (i == (length - 2) )
                        cout << line[bytePos[length -1]];
                }
            }
            cout << endl;
        }
    }
    else
        cout << "Error opening file" << endl;

    fileToRead.close();
}

/* Displaying the value of the fields based on parameter fields and delimeter. If delimiter isn't the
one in the file, it display whole line. */
void readFields(const string& fileName,char delim , const vector<int>& fields)
{
    string line;
    ifstream fileToRead (fileName.c_str());
    size_t length = fields.size();
    if (fileToRead.is_open())
    {
        while(getline(fileToRead,line))
        {
            vector<string> splitLine;
            splitLine.reserve(line.length());

            istringstream line_stream(line);
            string words;
            /* Separating fields based on provided delimiter and then storing them in a vector splitLine*/
            while(getline(line_stream,words,delim))
                splitLine.push_back(words);

            size_t splitSize = splitLine.size();
            if (line.find(delim) != string::npos)
            {
                if ((length == 1) && ( fields[0] < splitSize))
                    cout << splitLine[fields[0]];

                for(unsigned i = 0; i < length -1 ; ++i)
                {
                    /* Cases where we don't the exact end field position of a line, we loop until the end of the line.
                    Using END helps in those cases */
                    if (fields[i+1] == END)
                    {
                        int j = fields[i];
                        if (i==0)
                            cout << splitLine[j++];
                        while (j < splitSize-1 )
                        {
                            cout << delim << splitLine[j++];
                        }

                        if (j < splitSize)
                            cout << delim << splitLine[j];
                    }
                    else
                    {
                        /* In this case we are aware of the last field we need to extract */
                        if(fields[i] < splitSize)
                        {
                            if (i == 0)
                                cout << splitLine[fields[i]];
                            else
                                cout << delim << splitLine[fields[i]];

                            if ((i == (length - 2) ) && (fields[length -1] < splitSize))
                                cout << delim << splitLine[fields[length -1]];

                        }
                    }
                }
                cout << endl;
            }
            /* In case delimiter isn't the right one, display whole line*/
            else cout << line << endl;
        }
    }
    else
        cout << "Error opening file" << endl;

    fileToRead.close();
}

/* Parsing commands line parameters to generate bytes/fields needed for the output.
    This is done by generating bytes/fields position from arguments like -3,8,9-*/
int parseParameters(const string& param, vector<int>& fields)
{
    int res = RES_OK;
    istringstream param_stream(param);
    string split_param;
    char delimiter = ',';

    /* Parsing arguments delimited by , e.g. 1-2,4,5 */
    while(getline(param_stream,split_param,delimiter))
    {
        size_t pos = split_param.find("-");
        if ( pos != string::npos)
        {
            /* Dividing string of the format 1-2 or -2 or 1- in start_str and finish_str, where - is used as separation */
            string start_str = split_param.substr(0,pos);
            string finish_str = split_param.substr(pos+1);

            /* case1: where split_param is of type a-b*/
            if (!start_str.empty() && !finish_str.empty())
            {
                if (!is_number(start_str) || !is_number(finish_str) )
                {
                    res = BAD_INPUT;
                    return res;
                }
                int start = atoi(start_str.c_str()) - 1;
                int finish = atoi(finish_str.c_str()) - 1;

                while(start <= finish)
                {
                    fields.push_back(start);
                    ++start;
                }
            }
            /* case2: where split_param is of type -b */
            else if ( start_str.empty() && !finish_str.empty())
            {
                if (!is_number(finish_str))
                {
                    res = BAD_INPUT;;
                    return res;
                }
                int start = 0;
                int finish = atoi(finish_str.c_str()) -1;
                while (start <= finish)
                {
                   fields.push_back(start);
                   ++start;
                }
            }
            /* case3: where split_param is of type a- */
            else if ( !start_str.empty() && finish_str.empty() )
            {
                if (!is_number(start_str))
                {
                    res = BAD_INPUT;
                    return res;
                }
                int start = atoi(start_str.c_str()) - 1;
                fields.push_back(start);
                int finish = END;
                fields.push_back(finish);
            }
            /* case4: where split param is just - */
            else if (start_str.empty() && finish_str.empty())
            {
                int start = 0;
                fields.push_back(start);
                int finish = END;
                fields.push_back(finish);
            }

        }
        else
        {
            /* For split_param which contains just a single digit */
            if (!is_number(split_param))
            {
                res = BAD_INPUT;
                return res;
            }
            int col = atoi(split_param.c_str()) - 1;
            fields.push_back(col);
        }

    }
    return res;
}

/* Parsing commands line parameters to generate complements of bytes/fields needed for the output */
int parseComplementParameters (const string& param, vector<int>& cmplmnt_fields)
{
    int res = RES_OK;
    vector<int> fields;
    res = parseParameters(param,fields);
    if (res != RES_OK)
        return res;

    size_t vectLength = fields.size();

    /* complement of a single field */
    if (vectLength == 1)
    {
        if (fields[0] != 0)
        {
            int start = 0;
            int finish = fields[0] - 1;
            while (start <= finish)
                cmplmnt_fields.push_back(start++);
        }
        cmplmnt_fields.push_back(fields[0]+1);
        cmplmnt_fields.push_back(END);
    }
    /* complement for list like 3- after -c or -f options e.g. -c3- */
    else if ((vectLength == 2) && (fields[1] == END))
    {
        int start = 0;
        int finish = fields[0] -1;
        while (start <= finish)
            cmplmnt_fields.push_back(start++);
    }
    else
    {
        /* In rest of the cases, we try to generate all numbers between two non consecutive non negative positions.*/
        for(unsigned i = 0; i < vectLength-1; ++i)
        {
            if ((i==0) && (fields[i] != 0))
            {
                int start = 0;
                int finish = fields[i] - 1;
                while(start <= finish)
                    cmplmnt_fields.push_back(start++);
            }
            if (fields[i+1] != END)
            {
                int start = fields[i] + 1;
                int finish = fields[i+1] - 1;
                while( start <= finish )
                    cmplmnt_fields.push_back(start++);
                if (i+1 == vectLength - 1)
                {
                    cmplmnt_fields.push_back(fields[i+1]+1);
                    cmplmnt_fields.push_back(END);
                }
            }
        }
    }
    return res;
}

void help()
{
    cout << "SYNOPSIS" << endl;
    cout << "\tcut -c list file" << endl;
    cout << "\tcut -f list file" << endl;
    cout << "\tcut -d delimiter -f or -c list file" << endl;
    cout << "\tcut --complement -f or -c list file" << endl;
    cout << endl;
    cout << "USAGE" << endl;
    cout << "\tOnly allowed options after cut are -c, -f, -d and --complement" << endl;
    cout << "\tlist can be comma separated bytes or field position like 1,2,3 or 1-3,5- where - denotes the range " << endl;
    cout << "\tThere is no space between options and list e.g -c1,2,3 or -c-1,4- , where options can be -c or -f "<<endl;
    cout << "\tDelimiter is specified by -d and putting delimiter inside '' just after -d e.g. -d';', where ; is delimiter" <<endl;
    cout << "\tFor complement -c or -f should appear after --complement" << endl;
}
int main(int argc, char* argv[]) {

    try{
        int res = RES_OK; // checks the correctness of provided input
        string arg_help = argv[1];
        string help_option("--help");
        if (arg_help==help_option)
        {
            help();
            return 0;
        }
        else if (argc < 3)
        {
            cout<< "Insufficient number of arguments" << endl;
        }
        else
        {
            // Data file
            string fileName(argv[argc-1]);
            // Possible options that can be prp
            string byte_option("-c");
            string delim_option("-d");
            string field_option("-f");
            string complement_option("--complement");
            char delimiter = '\t';       // Delimiter with default value of TAB
            bool bComplement = false;   // Tracks whether we are looking for complement or not
            for(int i = 1; i < argc - 1; ++i)
            {
                string arg = argv[i];
                bool bIncorrect_opt = ((arg.find(byte_option)==string::npos) && (arg.find(delim_option)==string::npos)
                     && (arg.find(field_option)==string::npos) && (arg.find(complement_option)==string::npos));
                size_t argLen = arg.length();

                if ((argLen < 3) || bIncorrect_opt)
                {
                    cout << "Bad input size or options" << arg << endl;
                    break;
                }

                if (arg.compare(0,complement_option.size(),complement_option)==0)
                {
                    bComplement = true;
                }

                /* In case we try to take complement of whole of bytes or fields, which will be an empty output and hence we skip it */
                bool wrong_complement_option = (bComplement && (arg.length() == 3) && (arg[argLen-1] == '-'));
                bool wrong_complement_option_two = (bComplement && (arg.length()==4) && (arg.substr(2)== "1-"));
                if (wrong_complement_option ||wrong_complement_option_two)
                {
                    cout << "Bad input for complement option " << arg << endl;
                    break;
                }

                if (arg.compare(0,byte_option.size(),byte_option)==0)
                {
                    size_t pos = arg.find(byte_option);
                    vector<int> bytePos;
                    if (!bComplement)
                    {
                        res = parseParameters(arg.substr(pos+2),bytePos);
                        if (res != BAD_INPUT)
                            readBytes(fileName,bytePos);
                        else
                            cout << "Check the entered inputs, use --help option " << endl;
                    }
                    else
                    {
                        res = parseComplementParameters(arg.substr(pos+2),bytePos);
                        if (res!= BAD_INPUT)
                            readBytes(fileName,bytePos);
                        else
                            cout << "Check the entered inputs, use --help option" << endl;
                    }
                }
                if (arg.compare(0,delim_option.size(),delim_option)==0)
                {
                    delimiter = arg[3];
                }
                if (arg.compare(0,field_option.size(),field_option)==0)
                {
                    size_t pos = arg.find(field_option);
                    vector<int> fields;
                    if(!bComplement)
                    {
                        res = parseParameters(arg.substr(pos+2),fields);
                        if (res != BAD_INPUT)
                            readFields(fileName,delimiter,fields);
                        else
                            cout << "Check the entered inputs, use --help option" << endl;
                    }
                    else
                    {
                        res = parseComplementParameters(arg.substr(pos+2),fields);
                        if (res != BAD_INPUT)
                            readFields(fileName,delimiter,fields);
                        else
                            cout << "Check the entered inputs, use --help option" << endl;
                    }
                }
            }
        }
    }
    catch(const exception& e)
    {
        cout << e.what() << endl;
    }
    catch (...)
    {
        cout << "Unkown exception" << endl;
    }
    return 0;
}
