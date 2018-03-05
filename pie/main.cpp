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

int main(int argc, char **argv)
{
    pie::app::Application application(argc, argv);

    application.register_command(new pie::app::CommmandConstructor<pie::app::GavcCommand>("gavc", "GAVC query implementation."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::InitWorkingCopyCommand>("init", "Initialize working copy in current directory."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::CreateCommand>("create", "Create new tree."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::DestroyCommand>("destroy", "Destroy tree."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::CheckoutCommand>("checkout", "Checkout tree state into working copy."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::StatusCommand>("status", "Show working copy status in comparsion to referenced tree."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::ResetCommand>("reset", "Reset working copy content to the tree state."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::CommitCommand>("commit", "Commit tree state into current tree."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::TreeCommand>("tree", "Show tree names."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::LogCommand>("log", "Show tree log."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::PushCommand>("push", "Push to artifactory server."));
    application.register_command(new pie::app::CommmandConstructor<pie::app::UploadCommand>("upload", "Upload to artifactory server."));

    return application.run();
}
