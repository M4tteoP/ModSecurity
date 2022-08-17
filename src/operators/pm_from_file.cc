/*
 * ModSecurity, http://www.modsecurity.org/
 * Copyright (c) 2015 - 2021 Trustwave Holdings, Inc. (http://www.trustwave.com/)
 *
 * You may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * If any of the files related to licensing are missing or if you have any
 * other questions related to licensing please contact Trustwave Holdings, Inc.
 * directly using the email address security@modsecurity.org.
 *
 */

#include "src/operators/pm_from_file.h"

#include <string>

#include "src/operators/operator.h"
#include "src/utils/https_client.h"
#include "src/utils/system.h"


namespace modsecurity {
namespace operators {

bool PmFromFile::isComment(const std::string &s) {
    if (s.size() == 0) {
	  return true;
    }
    size_t pos = s.find("#");
    if (pos != std::string::npos) {
        for (int i = 0; i < pos; i++) {
	    if (!std::isspace(s[i])) {
		return false;
	    }
	}
    } else {
	return false;
    }

    return true;
}

// PmFromFile is currently not available for Wasm
// Tweaking it as a Pm with a custom delimiter.
// Example expected syntax: @PmFromFile delim:| htaccess|<script> <hello>|hi
bool PmFromFile::init(const std::string &config, std::string *error) {
    std::istream *iss;
    char delim;
    std::string tmp;

    if (m_param.compare(0, 6, "delim:") == 0 && m_param.size() > 7) {
        delim=m_param.at(6);
    } else {
        error->assign("Failed to find delimiter (e.g. 'delim:-') at:" + m_param);
        delete iss;
        return false;
    }

    m_param.erase(0,8);
    iss = new std::istringstream(m_param);

    while (std::getline(*iss, tmp, delim)) {
            acmp_add_pattern(m_p, tmp.c_str(), NULL, NULL, tmp.length());
    }

    // if (m_param.compare(0, 8, "https://") == 0) {
    //     Utils::HttpsClient client;
    //     bool ret = client.download(m_param);
    //     if (ret == false) {
    //         error->assign(client.error);
    //         return false;
    //     }
    //     iss = new std::stringstream(client.content);
    // } else {
    //     std::string err;
    //     std::string resource = utils::find_resource(m_param, config, &err);
    //     iss = new std::ifstream(resource, std::ios::in);

    //     if (((std::ifstream *)iss)->is_open() == false) {
    //         error->assign("Failed to open file: " + m_param + ". " + err);
    //         delete iss;
    //         return false;
    //     }
    // }

    // for (std::string line; std::getline(*iss, line); ) {
    //     if (isComment(line) == false) {
    //         acmp_add_pattern(m_p, line.c_str(), NULL, NULL, line.length());
	// }
    // }

    while (m_p->is_failtree_done == 0) {
        acmp_prepare(m_p);
    }

    delete iss;
    return true;
}


}  // namespace operators
}  // namespace modsecurity
