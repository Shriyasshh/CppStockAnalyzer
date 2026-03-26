#include<iostream>
#include<string>
#include<dotenv.h>
int main () {
  std::cout<<"Repo base init"<<std::endl;
  dotenv::inti();
  const char* env_key = std::getenv("API_KEY");
  std::string api_key = (env_key != nullptr) ? env_key : "KEY_NOT_FOUND";
  std::cout << "Securely loaded API Key: " << api_key << "\n\n"
  return 0;
}
