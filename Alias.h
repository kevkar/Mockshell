#pragma once

using namespace std;

struct Alias{
    string key;
    string value;
    string command;

    Alias(string k, string v, string c)
    {
        key = k;
        value = v;
        command = c;
    }

};