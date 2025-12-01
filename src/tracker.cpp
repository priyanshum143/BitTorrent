#include "torrent/tracker.hpp"
#include "torrent/bencode.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <sstream>
#include <vector>

namespace torrent {

    using json = nlohmann::json;


    // ----------------- HTTP Methods -----------------

    struct HttpResp {
        long status = 0;
        std::string body;
    };

    HttpResp http_get(const std::string& url) {
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("curl_easy_init failed");

        HttpResp r;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);

        auto rc = curl_easy_perform(curl);
        if (rc != CURLE_OK) {
            std::string err = curl_easy_strerror(rc);
            curl_easy_cleanup(curl);
            throw std::runtime_error("HTTP GET failed: " + err);
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r.status);
        curl_easy_cleanup(curl);
        return r;
    }


    // ----------------- URL Builder -----------------

    static std::string build_tracker_url(const TorrentMeta& meta, const std::string& peer_id) {
        std::string peer_id_enc = percent_encode_bytes(peer_id);
        std::string left = std::to_string(meta.length);

        std::ostringstream oss;
        oss << meta.announce
            << "?info_hash="  << meta.info_hash_urlencoded
            << "&peer_id="    << peer_id_enc
            << "&port="       << 6881
            << "&uploaded="   << 0
            << "&downloaded=" << 0
            << "&left="       << left
            << "&compact="    << 1
            << "&event=started";

        return oss.str();
    }


    // ----------------- Public API -----------------

    TrackerResponse request_peers(const TorrentMeta& meta, const std::string& peer_id) {
        TrackerResponse result;

        // 1. Build URL using TorrentMeta + peer_id
        std::string url = build_tracker_url(meta, peer_id);
        std::cerr << "[debug] URL: " << url << "\n";

        // 2. Perform HTTP GET (your real http_get will go here)
        HttpResp resp = http_get(url);

        if (resp.status != 200) {
            throw std::runtime_error("Tracker HTTP error: " + std::to_string(resp.status));
        }

        // 3. Bdecode tracker response
        json tr = decode_bencoded_value(resp.body);

        // 4. Failure reason from tracker
        if (tr.contains("failure reason")) {
            throw std::runtime_error("Tracker error: " + tr["failure reason"].get<std::string>());
        }

        // 5. Optional interval
        if (tr.contains("interval") && tr["interval"].is_number_integer()) {
            result.interval = tr["interval"].get<int>();
        }

        // 6. Compact peers ("peers" is a binary string)
        if (!tr.contains("peers")) {
            // no peers, return empty list
            return result;
        }

        std::string peers_bin = tr.at("peers").get<std::string>(); // binary-safe
        result.peers = parse_compact_peers(peers_bin);

        return result;
    }


    // ----------------- Compact peers parser -----------------

    // Each peer: 6 bytes: 4 for IP, 2 for port (big-endian).
    // So peers_bin.size() must be a multiple of 6.
    std::vector<Peer> parse_compact_peers(const std::string& peers_bin) {
        std::vector<Peer> out;

        if (peers_bin.size() % 6 != 0) {
            throw std::runtime_error("Invalid compact peers length");
        }

        for (std::size_t i = 0; i + 5 < peers_bin.size(); i += 6) {
            unsigned char b1 = static_cast<unsigned char>(peers_bin[i + 0]);
            unsigned char b2 = static_cast<unsigned char>(peers_bin[i + 1]);
            unsigned char b3 = static_cast<unsigned char>(peers_bin[i + 2]);
            unsigned char b4 = static_cast<unsigned char>(peers_bin[i + 3]);
            unsigned char p1 = static_cast<unsigned char>(peers_bin[i + 4]);
            unsigned char p2 = static_cast<unsigned char>(peers_bin[i + 5]);

            Peer peer;
            peer.ip = std::to_string(b1) + "." + std::to_string(b2) + "." +
                      std::to_string(b3) + "." + std::to_string(b4);
            peer.port = static_cast<std::uint16_t>((p1 << 8) | p2);

            out.push_back(peer);
        }

        return out;
    }
}
