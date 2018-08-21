/*
 * Copyright (c) 2018, Dmytro Iakovliev daemondzk@gmail.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Dmytro Iakovliev daemondzk@gmail.com ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Dmytro Iakovliev daemondzk@gmail.com BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <mavenpom.h>
#include <logging.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace pt = boost::property_tree;

namespace piel { namespace lib {

struct MavenPomConstants {
    static const std::string pom_modelVersion;
    static const std::string pom_modelVersion_ver;
    static const std::string pom_groupId;
    static const std::string pom_artifactId;
    static const std::string pom_version;
    static const std::string pom_packaging;
    static const std::string pom_packaging_pack;
    static const std::string pom_project;
};

const std::string MavenPomConstants::pom_modelVersion        = "modelVersion";
const std::string MavenPomConstants::pom_modelVersion_ver    = "4.0.0";
const std::string MavenPomConstants::pom_groupId             = "groupId";
const std::string MavenPomConstants::pom_artifactId          = "artifactId";
const std::string MavenPomConstants::pom_version             = "version";
const std::string MavenPomConstants::pom_project             = "project";
const std::string MavenPomConstants::pom_packaging           = "packaging";
const std::string MavenPomConstants::pom_packaging_pack      = "pom";

MavenPom::MavenPom()
    : group_()
    , name_()
    , version_()
{
}

MavenPom::~MavenPom()
{
}

std::string MavenPom::group() const
{
    return group_;
}

std::string MavenPom::name() const
{
    return name_;
}

std::string MavenPom::version() const
{
    return version_;
}

void MavenPom::set_group(const std::string& group)
{
    group_ = group;
}

void MavenPom::set_name(const std::string& name)
{
    name_ = name;
}

void MavenPom::set_version(const std::string& version)
{
    version_ = version;
}

/*static*/ MavenPom MavenPom::load(std::istream& is)
{
    MavenPom result;
    // TODO: Implement
    LOGF << "MavenPom::load is not implemented!" << ELOG;
    return result;
}

void MavenPom::store(std::ostream& os) const
{
    pt::ptree tree_project;
    pt::ptree tree;

    tree_project.put(MavenPomConstants::pom_modelVersion,    MavenPomConstants::pom_modelVersion_ver);
    tree_project.put(MavenPomConstants::pom_groupId,         group());
    tree_project.put(MavenPomConstants::pom_artifactId,      name());
    tree_project.put(MavenPomConstants::pom_version,         version());
    tree_project.put(MavenPomConstants::pom_packaging,       MavenPomConstants::pom_packaging_pack);

    tree.add_child(MavenPomConstants::pom_project, tree_project);

    pt::write_xml(os, tree);

    std::ostringstream oss;
    pt::write_xml(oss, tree);

    LOGT << "POM payload:" << ELOG;
    LOGT << oss.str() << ELOG;
}

} } // namespace piel::lib
