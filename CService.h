//
// Created by zielin on 01.11.23.
//

#ifndef STORAGE_CSERVICE_H
#define STORAGE_CSERVICE_H

#include <Poco/Util/ServerApplication.h>

class CService : public Poco::Util::ServerApplication
{
public:
	const char* name() const override;
	CService();

protected:
	void initialize(Application& self) override;
	void uninitialize() override;
	void reinitialize(Application& self) override;
	int main(const std::vector<std::string>& args) override;

    void defineOptions(Poco::Util::OptionSet &options) override;
    void handleConfig(const std::string &name, const std::string &value);
    void handleHelp(const std::string &name, const std::string &value);

	bool m_configLoaded = false;
};


#endif //STORAGE_CSERVICE_H
