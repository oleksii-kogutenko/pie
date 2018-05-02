/*
 * Copyright (c) 2017, Dmytro Iakovliev daemondzk@gmail.com
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

#include <application.h>
#include <gavccommand.h>
#include <initworkingcopycommand.h>
#include <createcommand.h>
#include <destroycommand.h>
#include <checkoutcommand.h>
#include <statuscommand.h>
#include <resetcommand.h>
#include <commitcommand.h>
#include <logcommand.h>
#include <treecommand.h>
#include <pushcommand.h>
#include <uploadcommand.h>
#include <pullcommand.h>
#include <configcommand.h>

namespace pa = pie::app;

int main(int argc, char **argv)
{
    pa::Application app(argc, argv);

#define C_(x,y,z) app.register_command( new pa::CommmandConstructor< x >( y, z ) )

    C_ ( pa::GavcCommand,               "gavc",     "GAVC query implementation." );
    C_ ( pa::UploadCommand,             "upload",   "Upload to Artifactory server." );
    C_ ( pa::PullCommand,               "pull",     "Pull from Artifactory server." );
    C_ ( pa::PushCommand,               "push",     "Push to Artifactory server." );

    C_ ( pa::InitWorkingCopyCommand,    "init",     "Initialize working copy in current directory." );
    C_ ( pa::CreateCommand,             "create",   "Create new tree." );
    C_ ( pa::DestroyCommand,            "destroy",  "Destroy tree." );
    C_ ( pa::CheckoutCommand,           "checkout", "Checkout the tree state into working copy." );
    C_ ( pa::StatusCommand,             "status",   "Show working copy status in compare with current tree state." );
    C_ ( pa::ResetCommand,              "reset",    "Reset working copy content to the tree state." );
    C_ ( pa::CommitCommand,             "commit",   "Commit current working copy state as new head tree state." );
    C_ ( pa::TreeCommand,               "tree",     "Show trees names." );
    C_ ( pa::LogCommand,                "log",      "Show tree log." );
    C_ ( pa::ConfigCommand,             "config",   "Setup configuration parameter." );

#undef C_

    return app.run();
}
