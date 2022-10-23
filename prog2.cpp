#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <ctime>
#include <fstream>
#include <pthread.h>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Data
{
public:
    Data(string c_d_i, vector<int> c_V, vector<string> m_d_i, vector<vector<int>> m_V, int num)
    {
        cur_doc_id = c_d_i;
        cur_V = c_V;
        my_doc_id = m_d_i;
        myV = m_V;
        number = num;
    }

    string get_cur_doc_id()
    {
        return cur_doc_id;
    }

    vector<int> get_cur_V()
    {
        return cur_V;
    }

    vector<string> get_my_doc_id()
    {
        return my_doc_id;
    }

    vector<vector<int>> get_myV()
    {
        return myV;
    }

    int get_number()
    {
        return number;
    }

    string get_max_doc_id()
    {
        return max_doc_id;
    }

    double get_max_avg_cos()
    {
        return max_avg_cos;
    }

    void set_max_doc_id(string new_max_doc_id)
    {
        max_doc_id = new_max_doc_id;
    }

    void set_max_avg_cos(double new_max_avg_cos)
    {
        max_avg_cos = new_max_avg_cos;
    }

private:
    static string max_doc_id;
    static double max_avg_cos;

    string cur_doc_id;
    vector<int> cur_V;
    vector<string> my_doc_id;
    vector<vector<int>> myV;
    int number;
};

string Data::max_doc_id = "";
double Data::max_avg_cos = 0;

void *compute(void *arg)
{
    clock_t start = clock();

    cout << "[TID=" << pthread_self() << "] DocID:" << (*((Data *)arg)).get_cur_doc_id() << " ";

    cout << "[" << (*((Data *)arg)).get_cur_V()[0];

    for (int i = 1; i < (*((Data *)arg)).get_cur_V().size(); i++)
    {
        cout << "," << (*((Data *)arg)).get_cur_V()[i];
    }

    cout << "]\n";

    vector<double> cos;

    for (int i = 0; i < (*((Data *)arg)).get_my_doc_id().size(); i++)
    {
        if ((*((Data *)arg)).get_number() == i)
        {
            continue;
        }

        double dot = 0;

        for (int j = 0; j < (*((Data *)arg)).get_cur_V().size(); j++)
        {
            dot += (*((Data *)arg)).get_cur_V()[j] * (*((Data *)arg)).get_myV()[i][j];
        }

        double pow1 = 0;

        for (int j = 0; j < (*((Data *)arg)).get_cur_V().size(); j++)
        {
            pow1 += pow((*((Data *)arg)).get_cur_V()[j], 2);
        }

        double sqrt1 = sqrt(pow1);

        double pow2 = 0;

        for (int j = 0; j < (*((Data *)arg)).get_cur_V().size(); j++)
        {
            pow2 += pow((*((Data *)arg)).get_myV()[i][j], 2);
        }

        double sqrt2 = sqrt(pow2);

        double mult = sqrt1 * sqrt2;

        double result = dot / mult;

        cout << "[TID=" << pthread_self() << "] cosine(" << (*((Data *)arg)).get_cur_doc_id() << ",";

        cout << (*((Data *)arg)).get_my_doc_id()[i] << ") = " << result << endl;

        cos.push_back(result);
    }

    double sum = 0;

    for (int i = 0; i < (*((Data *)arg)).get_my_doc_id().size() - 1; i++)
    {
        sum += cos[i];
    }

    double avg_cos = sum / ((*((Data *)arg)).get_my_doc_id().size() - 1);

    cout << "[TID=" << pthread_self() << "] Avg_cosine: " << avg_cos << endl;

    if (avg_cos > (*((Data *)arg)).get_max_avg_cos())
    {
        (*((Data *)arg)).set_max_doc_id((*((Data *)arg)).get_cur_doc_id());

        (*((Data *)arg)).set_max_avg_cos(avg_cos);
    }
    else if ((avg_cos == (*((Data *)arg)).get_max_avg_cos()) && ((*((Data *)arg)).get_cur_doc_id() < (*((Data *)arg)).get_max_doc_id()))
    {
        (*((Data *)arg)).set_max_doc_id((*((Data *)arg)).get_cur_doc_id());

        (*((Data *)arg)).set_max_avg_cos(avg_cos);
    }

    clock_t end = clock();

    double cpu_time = ((double)(end - start) / CLOCKS_PER_SEC);

    cout << "[TID=" << pthread_self() << "] CPU time: " << cpu_time * 1000 << " ms\n";

    return NULL;
}

int main(int argc, char *argv[])
{
    clock_t start = clock();

    vector<string> doc_id;

    vector<string> doc_content;

    string file_name = argv[1];

    ifstream ifs(file_name, ios::in);

    if (!ifs.is_open())
    {
        cout << "Failed to open file.\n";
    }
    else
    {
        string buffer;

        while (getline(ifs, buffer))
        {
            for (int i = 0; i < buffer.size(); i++)
            {
                if (ispunct(buffer[i]))
                {
                    buffer[i] = ' ';
                }
            }

            doc_id.push_back(buffer);

            getline(ifs, buffer);

            for (int i = 0; i < buffer.size(); i++)
            {
                if (ispunct(buffer[i]))
                {
                    buffer[i] = ' ';
                }
            }

            doc_content.push_back(buffer);
        }

        ifs.close();

        vector<string> words;

        for (int i = 0; i < doc_content.size(); i++)
        {
            stringstream ss;

            ss.str(doc_content[i]);

            string temp;

            while (true)
            {
                ss >> temp;

                if (ss.fail())
                {
                    break;
                }

                bool all_eng = true;

                for (int j = 0; j < temp.size(); j++)
                {
                    if (!isalpha(temp[j]))
                    {
                        all_eng = false;

                        break;
                    }
                }

                if (find(words.begin(), words.end(), temp) == words.end() && all_eng)
                {
                    words.push_back(temp);
                }
            }
        }

        vector<vector<int>> V;

        V.assign(doc_id.size(), vector<int>(words.size()));

        for (int i = 0; i < doc_content.size(); i++)
        {
            stringstream ss;

            ss.str(doc_content[i]);

            string temp;

            while (true)
            {
                ss >> temp;

                if (ss.fail())
                {
                    break;
                }

                auto it = find(words.begin(), words.end(), temp);

                if (it != words.end())
                {
                    int j = it - words.begin();

                    V[i][j]++;
                }
            }
        }

        vector<Data> args;

        for (int i = 0; i < doc_id.size(); i++)
        {
            Data temp_data(doc_id[i], V[i], doc_id, V, i);

            args.push_back(temp_data);
        }

        pthread_t threads[doc_id.size()];

        for (int i = 0; i < doc_id.size(); i++)
        {
            if (pthread_create(&threads[i], NULL, compute, (void *)&args[i]) != 0)
            {
                cout << "Fail to create pthread.\n";
            }
            else
            {
                cout << "[Main thread]:create TID:" << threads[i] << ", DocID:" << doc_id[i] << endl;
            }
        }

        for (int i = 0; i < doc_id.size(); i++)
        {
            pthread_join(threads[i], NULL);
        }

        cout << "[Main thread] KeyDocID:" << args[0].get_max_doc_id() << " Highest Average Cosine: " << args[0].get_max_avg_cos() << endl;
    }

    ifs.close();

    clock_t end = clock();

    double cpu_time = ((double)(end - start) / CLOCKS_PER_SEC);

    cout << "[Main thread] CPU time: " << cpu_time * 1000 << " ms\n";

    return 0;
}