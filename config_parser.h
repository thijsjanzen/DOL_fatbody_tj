//
//  config_params.h
//
//  From: https://www.dreamincode.net/forums/topic/183191-create-a-simple-configuration-file-parser/
//

#ifndef config_params_h
#define config_params_h

#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <utility>

struct config_err {
    void exitWithError(const std::string &error) {
        std::cout << error;
        std::cin.ignore();
        std::cin.get();

        exit(EXIT_FAILURE);
    }
};


struct Convert {
    // Convert T, which should be a primitive, to a std::string.
    template <typename T>
    static std::string T_to_string(T const &val) {
        std::ostringstream ostr;
        ostr << val;

        return ostr.str();
    }

    // Convert a std::string to T.
    template <typename T>
    static T string_to_T(std::string const &val) {
        config_err err;
        std::istringstream istr(val);
        T returnVal;
        if (!(istr >> returnVal))
            err.exitWithError("config: Not a valid " +
                              (std::string)typeid(T).name() + " received!\n");


        return returnVal;
    }

    static std::string string_to_T(std::string const &val) {
        return val;
    }
};

class ConfigFile {
      private:
        std::map<std::string, std::string> contents;
        std::string fName;
        config_err err;

        void removeComment(std::string &line) const {
            if (line.find('#') != line.npos)
                line.erase(line.find('#'));
        }

        bool onlyWhitespace(const std::string &line) const {
            return (line.find_first_not_of(' ') == line.npos);
        }

        bool validLine(const std::string &line) const {
            std::string temp = line;
            temp.erase(0, temp.find_first_not_of("\t "));
            if (temp[0] == '=')
                return false;

            for (size_t i = temp.find('=') + 1; i < temp.length(); i++)
                if (temp[i] != ' ')
                    return true;

            return false;
        }

        void extractKey(std::string &key,
                        size_t const &sepPos,
                        const std::string &line) const {
            key = line.substr(0, sepPos);
            if (key.find('\t') != line.npos || key.find(' ') != line.npos)
                key.erase(key.find_first_of("\t "));
        }

        void extractValue(std::string &value,
                          size_t const &sepPos,
                          const std::string &line) const {
            value = line.substr(sepPos + 1);
            value.erase(0, value.find_first_not_of("\t "));
            value.erase(value.find_last_not_of("\t ") + 1);
        }

        void extractContents(const std::string &line) {
            std::string temp = line;
            // Erase leading whitespace from the line.
            temp.erase(0, temp.find_first_not_of("\t "));
            size_t sepPos = temp.find('=');

            std::string key, value;
            extractKey(key, sepPos, temp);
            extractValue(value, sepPos, temp);

            if (!keyExists(key))
                contents.insert(
                    std::pair<std::string, std::string>(key, value));
            else
                err.exitWithError("config: Can only have unique key names!\n");
        }

        // lineNo = the current line number in the file.
        // line = the current line, with comments removed.
        void parseLine(const std::string &line, size_t const lineNo) {
          auto x = line.find('[');
          if (x == 0) {
            return;
          }

            if (line.find('=') == line.npos)
                err.exitWithError("config: Couldn't find separator on line: " +
                                  Convert::T_to_string(lineNo) + "\n");

            if (!validLine(line))
                err.exitWithError("config: Bad format for line: " +
                                  Convert::T_to_string(lineNo) + "\n");

            extractContents(line);
        }

        void ExtractKeys() {
            std::ifstream file;
            file.open(fName.c_str());
            if (!file)
                err.exitWithError("config: File " + fName +
                                  " couldn't be found!\n");

            std::string line;
            size_t lineNo = 0;
            while (std::getline(file, line)) {
                lineNo++;
                std::string temp = line;

                if (temp.empty())
                    continue;

                removeComment(temp);
                if (onlyWhitespace(temp))
                    continue;

                parseLine(temp, lineNo);
            }

            file.close();
        }



     public:
        ConfigFile(const std::string &fName) {
            this->fName = fName;
            ExtractKeys();
        }

        bool keyExists(const std::string &key) const {
            return contents.find(key) != contents.end();
        }

        template <typename ValueType>
        ValueType getValueOfKey(const std::string &key,
                           ValueType const &defaultValue = ValueType()) const {
            if (!keyExists(key))
                return defaultValue;

            ValueType output =
                Convert::string_to_T<ValueType>(contents.find(key)->second);
            std::cout << key << " = " << output << "\n";
            return output;
        }
};

#endif /* config_params_h */
