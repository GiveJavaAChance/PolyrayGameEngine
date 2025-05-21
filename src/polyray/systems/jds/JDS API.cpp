#define DISCORDPP_IMPLEMENTATION
#include "jds_JDS.h"
#include "discordpp.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <functional>
#include <csignal>

uint64_t applicationId;

std::shared_ptr<discordpp::Client> client;

std::atomic<bool> running = true;

void signalHandler(int signum) {
    running.store(false);
}

void displayFriendsList() {
    std::vector<std::string> relationships{};
    for (auto& relationship: client->GetRelationships()) {
        auto user = relationship.User();
        if (!user) {
            continue;
        }

        std::string str{};
        // Identifying information about the user:
        str += " DiscordName: " + user->DisplayName();
        str += " DiscordId: " + std::to_string(user->Id());
        // Provisional users don't have a Discord icon shown next to them:
        str += " IsProvisional: " + std::to_string(user->IsProvisional());
        // Whether the relationship is for a friend, a friend request, or because the user is blocked:
        // For a friends list you'll want to filter out blocked users
        // And likely display friend requests in a different section
        str += " DiscordRelationshipType: " + std::string(discordpp::EnumToString(relationship.DiscordRelationshipType()));
        str += " GameRelationshipType: " + std::string(discordpp::EnumToString(relationship.GameRelationshipType()));
        // Whether the user is online/offline/etc:
        str += " IsOnlineAnywhere: " + std::to_string(user->Status() != discordpp::StatusType::Offline);
        str += " IsOnlineInGame: " + std::to_string(user->GameActivity() != std::nullopt);
        relationships.push_back(str);
    }

    std::sort(relationships.begin(), relationships.end());
    for (auto str : relationships) {
        printf("%s\n", str.c_str());
    }
}

JNIEXPORT void JNICALL Java_jds_JDS_setup (JNIEnv* env, jobject obj, jlong a) {

    applicationId = static_cast<uint64_t>(a);
    std::signal(SIGINT, signalHandler);
    std::cout << "Initializing Discord SDK...\n";

    client = std::make_shared<discordpp::Client>();

    client->SetStatusChangedCallback([env, obj, client](discordpp::Client::Status status, discordpp::Client::Error error, int32_t errorDetail) {
        std::cout << "Status changed: " << discordpp::Client::StatusToString(status) << std::endl;
        if (status == discordpp::Client::Status::Ready) {
            std::cout << "Client is ready! You can now call SDK functions.\n";

            std::cout << "Friends Count: " << client->GetRelationships().size() << std::endl;

            displayFriendsList();

            discordpp::Activity activity;
            activity.SetType(discordpp::ActivityTypes::Playing);
            activity.SetState("Playing 237 offline");
            activity.SetDetails("Day 0, In overworld");

            client->UpdateRichPresence(activity, [](discordpp::ClientResult result) {
                if(result.Successful()) {
                    std::cout << "Rich Presence updated successfully!\n";
                } else {
                    std::cerr << "Rich Presence update failed";
                }
            });

            jclass jcls = env->FindClass("jds/JDS");
            if (jcls == nullptr) {
                std::cerr << "Could not find class jds.JDS\n";
                return;
            }

            jfieldID readyField = env->GetStaticFieldID(jcls, "READY", "Z");
            if (readyField == nullptr) {
                std::cerr << "Could not find field READY\n";
                return;
            }

            env->SetStaticBooleanField(jcls, readyField, JNI_TRUE);
        } else if (error != discordpp::Client::Error::None) {
            std::cerr << "Connection Error: " << discordpp::Client::ErrorToString(error) << " - Details: " << errorDetail << std::endl;
        }
    });

    auto codeVerifier = client->CreateAuthorizationCodeVerifier();

    discordpp::AuthorizationArgs args{};
    args.SetClientId(applicationId);
    args.SetScopes(discordpp::Client::GetDefaultPresenceScopes());
    args.SetCodeChallenge(codeVerifier.Challenge());

    client->Authorize(args, [client, codeVerifier](auto result, auto code, auto redirectUri) {
        if (!result.Successful()) {
            std::cerr << "Authentication Error: " << result.Error() << std::endl;
            return;
        } else {
            std::cout << "Authorization successful! Getting access token...\n";

            client->GetToken(applicationId, code, codeVerifier.Verifier(), redirectUri,
            [client](discordpp::ClientResult result,
            std::string accessToken,
            std::string refreshToken,
            discordpp::AuthorizationTokenType tokenType,
            int32_t expiresIn,
            std::string scope) {
                std::cout << "Access token received! Establishing connection...\n";

                client->UpdateToken(discordpp::AuthorizationTokenType::Bearer,  accessToken, [client](discordpp::ClientResult result) {
                    if(result.Successful()) {
                        std::cout << "Token updated, connecting to Discord...\n";
                        client->Connect();
                    }
                });
            });
        }
    });
}

JNIEXPORT void JNICALL Java_jds_JDS_setActivity (JNIEnv* env, jobject, jstring a, jstring b) {
    const char* activityState = env->GetStringUTFChars(a, nullptr);
    const char* activityDetails = env->GetStringUTFChars(b, nullptr);

    discordpp::Activity activity;
    activity.SetState(activityState);
    activity.SetDetails(activityDetails);

    client->UpdateRichPresence(activity, [](discordpp::ClientResult result) {
        if (result.Successful()) {
            std::cout << "Rich Presence updated successfully!\n";
        } else {
            std::cerr << "Rich Presence update failed\n";
        }
    });

    env->ReleaseStringUTFChars(a, activityState);
    env->ReleaseStringUTFChars(b, activityDetails);
}

JNIEXPORT void JNICALL Java_jds_JDS_update (JNIEnv* env, jobject) {
    discordpp::RunCallbacks();
}
