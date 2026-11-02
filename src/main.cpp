
// oh no i just realised that this mod arleady exists :sob: (hopefully its for 2.207)

#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/CustomSongWidget.hpp>

class $modify(SECustomSongWidget, CustomSongWidget) {
	
	void showErrPopup(const char * err) {
		FLAlertLayer::create(
			"Song Export",
			"An error occured while exporting a song: " + std::string(err),
			"OK"
		)->show();
	}

	std::string safeName(std::string sogName) {
		for (char& c : sogName) {
			std::string bad = "\\/:*?\"<>|";
			if (bad.find(c) != std::string::npos) c = '_';
		}
		return sogName;
	};
	void exportSong(CCObject* sender) {
		return;
		std::string fname = "";
		std::error_code err;
		if (m_songDelegate && m_songDelegate->getSongFileName()!="") {
			fname = m_songDelegate->getSongFileName();
			log::info("Got Song filename from m_songDelegate, songID is {}", m_customSongID);
		} 
		else if (m_customSongID && std::filesystem::exists(MusicDownloadManager::sharedState()->pathForSong(m_customSongID), err) && !err) {
			fname = MusicDownloadManager::sharedState()->pathForSong(m_customSongID);
			log::info("Got Song filename from res dir ({}), songID is {}", fname, m_customSongID);
		} 
		else {
			log::error("Can't find song filename");
			showErrPopup("Cannot find song file.");
			return;
		}

		asp::fs::Path fname_as_path(fname);
		asp::fs::Path def_path = fname_as_path;

		if (fname_as_path.hasExtension() && m_songInfoObject && m_songInfoObject->m_songName!="") {
			geode::log::info("song name is {}" ,m_songInfoObject->m_songName);
			def_path = safeName(m_songInfoObject->m_songName) + std::string(fname_as_path.extension().value());
		} 

		auto exp_opts = file::FilePickOptions{
			fname_as_path, 
			{{"Audio File", {"*.mp3", "*.m4a", "*.ogg", "*.opus", "*.oga", "*.flac", "*.wav", "*.aiff", "*.aif" }}}
		};
		
		log::debug("def_path={}", def_path);
		exp_opts.defaultPath = def_path;
		


		async::spawn(
			file::pick(file::PickMode::SaveFile, exp_opts),
			[this, fname, fname_as_path](file::PickResult fres){
				if (!fres.ok()) {
					log::error("error with picker result - {}", fres.err());
					showErrPopup((fres.err()->c_str()));
					return;
				}
				auto trg_path = std::move(fres).unwrap();
				if (!trg_path) {
					log::error("err - empty path");
					showErrPopup("Empty path");
					return;
				}
				auto trg_path_uw = std::move(trg_path).value(); 
				auto write_res = asp::fs::copy(fname_as_path, trg_path_uw);
				if (write_res.isErr()) {
					log::error("error while copying - {}", write_res.unwrapErr());
					showErrPopup(std::string(write_res.unwrapErr()).c_str());
					return;
				}
				
				log::info("Copied file from {} to {}", fname, trg_path_uw);
				
				FLAlertLayer::create(
					"Song Export",
					"Song Was Exported!",
					"OK"
				)->show();
				log::info("Song was Exported!");
			}
		);
	}

	

	bool init(SongInfoObject* songInfo, CustomSongDelegate* songDelegate, bool showSongSelect, bool showPlayMusic, bool showDownload, bool isRobtopSong, bool unkBool, bool isMusicLibrary, int unk, bool p9, bool p10) {
		if (!CustomSongWidget::init(songInfo, songDelegate, showSongSelect, showPlayMusic, showDownload, isRobtopSong, unkBool, isMusicLibrary, unk, p9, p10)) return false;

		CCMenu* expMenu = CCMenu::create();
		expMenu->setID("song-export-menu"_spr);
		//log::info("unk bool ={}; unkINT={}", unkBool, unk);

		//auto expBtnSpr = CCSprite::create("export-btn.png"_spr);
		auto expBtnSpr = CircleButtonSprite::createWithSpriteFrameName("GJ_downloadsIcon_001.png", 0.8, geode::CircleBaseColor::Blue, geode::CircleBaseSize::Tiny);
		auto expBtn = CCMenuItemSpriteExtra::create(expBtnSpr, this, menu_selector(SECustomSongWidget::exportSong));

		expBtn->setID("song-export-btn");
		bool hasCopySogIdBtn = false;
		if (m_buttonMenu && m_buttonMenu->getChildByID("raydeeux.copysongid/copy-song-id")) hasCopySogIdBtn = true;
		if (unk==1){ // if in "audio assets" menu (in "compact mode") ig
			expMenu->setContentSize(ccp(320, 50)); // smol
		} else if (hasCopySogIdBtn) {
			expMenu->setContentSize(ccp(300, 35)); // big, a little lower
		} else {
			expMenu->setContentSize(ccp(300, 90)); // big
		}
		if (!hasCopySogIdBtn || unk!=1) expMenu->addChildAtPosition(expBtn, Anchor::TopRight);
		else expMenu->addChildAtPosition(expBtn, Anchor::BottomRight);

		this->addChildAtPosition(expMenu, Anchor::Center);

		return true;
	}
};


// the the end
