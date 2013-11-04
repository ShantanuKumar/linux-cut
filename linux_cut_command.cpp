// Implements reduced version of linux command line tool cut
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#define END -1

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
            while(getline(line_stream,words,delim))
                splitLine.push_back(words);

            if (line.find(delim) != string::npos)
            {
                if (length == 1)
                    cout << splitLine[fields[0]];

                for(unsigned i = 0; i < length -1 ; ++i)
                {
                    if (fields[i+1] == END)
                    {
                        int j = fields[i];
                        while (j < splitLine.size()-1 )
                            cout << splitLine[j++] << delim;

                        if (j < splitLine.size())
                            cout << splitLine[j];
                    }
                    else
                    {
                        if(fields[i] < splitLine.size())
                        {
                            cout << splitLine[fields[i]] << delim;
                            if ((i == (length - 2) ) && (fields[length -1] < splitLine.size()))
                                cout << splitLine[fields[length -1]];
                        }
                    }
                }
                cout << endl;
            }
            else cout << line << endl;
        }
    }
    else
        cout << "Error opening file" << endl;

    fileToRead.close();
}

/* Parsing commands line parameters to generate bytes/fields needed for the output */
void parseParameters(const string& param, vector<int>& fields)
{
    istringstream param_stream(param);
    string split_param;
    char delimiter = ',';
    while(getline(param_stream,split_param,delimiter))
    {
        size_t pos = split_param.find("-");
        if ( pos != string::npos)
        {
            if (split_param.length()==3)
            {
                int start = split_param[0] - 49;
                int finish = split_param[2] - 49;
                while(start <= finish)
                {
                    fields.push_back(start);
                    ++start;
                }
            }
            else if (split_param.length()==2)
            {
                if (pos == 0)
                {
                   int start = 0;
                   int finish = split_param[1] - 49;
                   while (start <= finish)
                   {
                       fields.push_back(start);
                       ++start;
                   }
                }
                else
                {
                    int start = split_param[0] - 49;
                    fields.push_back(start);
                    int finish = END;
                    fields.push_back(finish);
                }
            }
            else
            {
                int start = 0;
                fields.push_back(start);
                int finish = END;
                fields.push_back(finish);
            }

        }
        else
        {
            int col = split_param[0] - 49;
            fields.push_back(col);
        }

    }
}

/* Parsing commands line parameters to generate complements of bytes/fields needed for the output */
void parseComplementParameters (const string& param, vector<int>& cmplmnt_fields)
{
    vector<int> fields;
    parseParameters(param,fields);
    size_t vectLength = fields.size();

    if (vectLength == 1)
    {
        cmplmnt_fields.push_back(fields[0]+1);
        cmplmnt_fields.push_back(END);
    }
    else if ((vectLength == 2) && (fields[1] == END))
    {
        int start = 0;
        int finish = fields[0] -1;
        while (start <= finish)
            cmplmnt_fields.push_back(start++);
    }
    else
    {
        for(unsigned i = 0; i < vectLength-1; ++i)
        {
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

}
int main(int argc, char* argv[]) {

    try{

        if (argc < 3)
        {
            cout<< "Insufficient number of arguments" << endl;
        }
        else
        {
            // Data file
            string fileName(argv[argc-1]);

            // Possible options
            string byte_option("-c");
            string delim_option("-d");
            string field_option("-f");
            string complement_option("--complement");
            char delimiter = ';';       // Delimiter with default value of TAB
            bool bComplement = false;   // Tracks whether we are looking for complement or not
            for(int i = 1; i < argc - 1; ++i)
            {
                string arg = argv[i];

                // Checking whether correct option has been provided or not
                bool bIncorrect_opt = ((arg.find(byte_option)==string::npos) && (arg.find(delim_option)==string::npos)
                     && (arg.find(field_option)==string::npos) && (arg.find(complement_option)==string::npos));
                if ((arg.length() < 3) || bIncorrect_opt)
                {
                    cout << "Bad input " << arg << endl;
                    break;
                }

                if (arg.compare(0,complement_option.size(),complement_option)==0)
                {
                    bComplement = true;
                }
                if (arg.compare(0,byte_option.size(),byte_option)==0)
                {
                    size_t pos = arg.find(byte_option);
                    vector<int> bytePos;
                    if (!bComplement)
                    {
                        parseParameters(arg.substr(pos+2),bytePos);
                        readBytes(fileName,bytePos);
                    }
                    else if (arg[2] != '-')
                    {
                        parseComplementParameters(arg.substr(pos+2),bytePos);
                        readBytes(fileName,bytePos);
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
                        parseParameters(arg.substr(pos+2),fields);
                        readFields(fileName,delimiter,fields);
                    }

                    else if (arg[2] != '-')
                    {
                        parseComplementParameters(arg.substr(pos+2),fields);
                        readFields(fileName,delimiter,fields);
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
