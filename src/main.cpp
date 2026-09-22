
// oh no i just realised that this mod arleady exists :sob:

#include "Geode/loader/Log.hpp"
#include <Geode/Geode.hpp>
#include <filesystem>

using namespace geode::prelude;

#include <Geode/modify/CustomSongWidget.hpp>

class $modify(SECustomSongWidget, CustomSongWidget) {
	void showErrPopup(const char * err) {
		FLAlertLayer::create(
			"Song Export",
			"An error occured while exporting a song: " + fmt::to_string(err),
			"OK"
		)->show();
	}
	std::string getSongFilenameExtensionViaID(int sogID) { // the shitcode
		auto pathPrefix = (dirs::getResourcesDir() / (std::to_string(sogID)));
		if (std::filesystem::exists(pathPrefix.string() + ".mp3")) {
			return ".mp3";
		} else if (std::filesystem::exists(pathPrefix.string() + ".ogg")) {
			return ".ogg";
		} else if (std::filesystem::exists(pathPrefix.string() + ".m4a")) {
			return ".m4a";
		} else if (std::filesystem::exists(pathPrefix.string() + ".opus")) {
			return ".opus";
		} else if (std::filesystem::exists(pathPrefix.string() + ".oga")) {
			return ".oga";
		} else if (std::filesystem::exists(pathPrefix.string() + ".flac")) {
			return ".flac";
		} else if (std::filesystem::exists(pathPrefix.string() + ".wav")) {
			return ".wav";
		} else if (std::filesystem::exists(pathPrefix.string() + ".aiff")) {
			return ".aiff";
		} else if (std::filesystem::exists(pathPrefix.string() + ".aif")) {
			return ".aif";
		} else return "";
	}
	std::string safeName(std::string sogName) {
		for (char& c : sogName) {
			std::string bad = "\\/:*?\"<>|";
			if (bad.find(c) != std::string::npos) c = '_';
		}
		return sogName;
	};
	void exportSong(CCObject* sender) {

		gd::string fname = "";

		if (m_songDelegate && m_songDelegate->getSongFileName()!="") {
			fname = m_songDelegate->getSongFileName();
			log::info("Got Song filename from m_songDelegate, songID is {})", m_customSongID);
		} 
		else if (m_customSongID && std::filesystem::exists((dirs::getResourcesDir() / (std::to_string(m_customSongID) + getSongFilenameExtensionViaID(m_customSongID))))) {
			fname = (dirs::getResourcesDir() / (std::to_string(m_customSongID) + getSongFilenameExtensionViaID(m_customSongID))).string();
			log::info("Got Song filename from res dir ({}), songID is {}", fname, m_customSongID);
		} 
		else {
			log::error("Can't find song filename");
			showErrPopup("Cannot find song file");
			return;
		}

		std::filesystem::path fname_as_path(fname);
		std::filesystem::path def_path = fname_as_path;

		if (fname_as_path.has_extension() && m_songInfoObject && m_songInfoObject->m_songName!="") {
			geode::log::info("song namne is {}" ,m_songInfoObject->m_songName);
			def_path = safeName(m_songInfoObject->m_songName) + fname_as_path.extension().string();
		} 

		auto exp_opts = file::FilePickOptions{
			fname_as_path, 
			{{"Audio File", {"*.mp3", "*.m4a", "*.ogg", "*.opus", "*.oga", "*.flac", "*.wav", "*.aiff", "*.aif" }}}
		};
		
		log::debug("def_path={}", def_path);
		exp_opts.defaultPath = def_path;
		


		async::spawn(
			file::pick(file::PickMode::SaveFile, exp_opts),
			[this, fname](file::PickResult fres){
				if (!fres.ok()) {
					log::error("error with picker result - ", fres.err());
					showErrPopup((fres.err()->c_str()));
					return;
				}
				auto path = std::move(fres).unwrap();
				if (!path) {
					log::error("err - empty path");
					showErrPopup("Empty path");
					return;
				}
				auto path_str = std::move(path).value();
				log::info("Copied file from {} to {}", fname, path_str);

				try {
					std::filesystem::copy(fname, path_str);
					log::info("Copied file from {} to {}", fname, path_str);
				} catch (std::exception& e) {
					log::error("error while copying - ", e.what());
					showErrPopup(e.what());
					return;
				}

				FLAlertLayer::create(
					"Song Export",
					"Song Was Exported!",
					"OK"
				)->show();
				log::info("Song was Exported!");
			}
		);
	}

	bool init(SongInfoObject* songInfo, CustomSongDelegate* songDelegate, bool showSongSelect, bool showPlayMusic, bool showDownload, bool isRobtopSong, bool unkBool, bool isMusicLibrary, int unk) {
		if (!CustomSongWidget::init(songInfo, songDelegate, showSongSelect, showPlayMusic, showDownload, isRobtopSong, unkBool, isMusicLibrary, unk)) return false;

		CCMenu* expMenu = CCMenu::create();
		expMenu->setID("song-export-menu"_spr);
		//log::info("unk bool ={}; unkINT={}", unkBool, unk);

		auto expBtnSpr = CCSprite::create("export-btn.png"_spr);
		auto expBtn = CCMenuItemSpriteExtra::create(expBtnSpr, this, menu_selector(SECustomSongWidget::exportSong));

		expBtn->setID("song-export-btn");
		if (unk==1){ // if in "audio assets" menu (in "compact mode") ig
			expMenu->setContentSize(ccp(320, 50)); // smol
		} else {
			expMenu->setContentSize(ccp(300, 90)); // big
		}
		expMenu->addChildAtPosition(expBtn, Anchor::TopRight);

		this->addChildAtPosition(expMenu, Anchor::Center);

		return true;
	}
};

// the the end