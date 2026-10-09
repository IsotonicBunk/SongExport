
// oh no i just realised that this mod arleady exists :sob: (hopefully its for 2.207)

#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include <Geode/modify/CustomSongWidget.hpp>

class $modify(SECustomSongWidget, CustomSongWidget) {
	std::filesystem::path getSongPath(int sogId) {
		std::filesystem::path path = dirs::getResourcesDir();
		if (isThereASongFileExistsAtTsPathWithTsId(sogId, path)) return path / (std::to_string(sogId) + getSongFilenameExtensionViaIDAndPath(sogId, path));
		
		path = dirs::getSaveDir();
		if (isThereASongFileExistsAtTsPathWithTsId(sogId, path)) return path / (std::to_string(sogId) + getSongFilenameExtensionViaIDAndPath(sogId, path));

		auto home_str = std::getenv("HOME"); // this probably should be the home dir on macos
		if (!home_str) return "";

		std::filesystem::path home_pth = std::move(home_str);
		path = home_pth / "Library" / "Caches"; // dont sure if it will work
		if (isThereASongFileExistsAtTsPathWithTsId(sogId, path)) return path / (std::to_string(sogId) + getSongFilenameExtensionViaIDAndPath(sogId, path));
		return "";
	}
	void showErrPopup(const char * err) {
		FLAlertLayer::create(
			"Song Export",
			"An error occured while exporting a song: " + std::string(err),
			"OK"
		)->show();
	}

	bool isThereASongFileExistsAtTsPathWithTsId(int sogId, std::filesystem::path path) {
		if (getSongFilenameExtensionViaIDAndPath(sogId, path) == "") return false;
		return true;
	}
	std::string getSongFilenameExtensionViaIDAndPath(int sogID, std::filesystem::path path) { // the shitcode
		auto pathPrefix = utils::string::pathToString(path / std::to_string(sogID));
		std::error_code err;
		if (std::filesystem::exists(pathPrefix + ".mp3", err) && !err) {
			return ".mp3";
		} else if (std::filesystem::exists(pathPrefix + ".ogg", err) && !err) {
			return ".ogg";
		} else if (std::filesystem::exists(pathPrefix + ".m4a", err) && !err) {
			return ".m4a";
		} else if (std::filesystem::exists(pathPrefix + ".opus", err) && !err) {
			return ".opus";
		} else if (std::filesystem::exists(pathPrefix + ".oga", err) && !err) {
			return ".oga";
		} else if (std::filesystem::exists(pathPrefix + ".flac", err) && !err) {
			return ".flac";
		} else if (std::filesystem::exists(pathPrefix + ".wav", err) && !err) {
			return ".wav";
		} else if (std::filesystem::exists(pathPrefix + ".aiff", err) && !err) {
			return ".aiff";
		} else if (std::filesystem::exists(pathPrefix + ".aif", err) && !err) {
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
		std::error_code err;
		if (m_songDelegate && m_songDelegate->getSongFileName()!="") {
			fname = m_songDelegate->getSongFileName();
			log::info("Got Song filename from m_songDelegate, songID is {}", m_customSongID);
		} 
		else if (m_customSongID && std::filesystem::exists(getSongPath(m_customSongID), err) && !err) {
			fname = utils::string::pathToString(getSongPath(m_customSongID));
			log::info("Got Song filename from res dir ({}), songID is {}", fname, m_customSongID);
		} 
		else {
			log::error("Can't find song filename");
			showErrPopup("Cannot find song file.");
			return;
		}

		std::filesystem::path fname_as_path(fname);
		std::filesystem::path def_path = fname_as_path;

		if (fname_as_path.has_extension() && m_songInfoObject && m_songInfoObject->m_songName!="") {
			geode::log::info("song name is {}" ,m_songInfoObject->m_songName);
			def_path = safeName(m_songInfoObject->m_songName) + utils::string::pathToString(fname_as_path.extension());
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

				auto fdata = file::readBinary(fname_as_path); // was using filesystem::copy earlier, 
				if (fdata.isErr()) {                                                    // until i realized that it crashes on android for some reason
					log::error("error while reading - {}", fdata.unwrapErr());
					showErrPopup(fdata.unwrapErr().c_str());
					return;
				}
				
				auto write_res = file::writeBinarySafe(trg_path_uw, std::move(fdata).unwrap());
				if (write_res.isErr()) {
					log::error("error while writing - {}", write_res.unwrapErr());
					showErrPopup(write_res.unwrapErr().c_str());
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

	bool init(SongInfoObject* songInfo, CustomSongDelegate* songDelegate, bool showSongSelect, bool showPlayMusic, bool showDownload, bool isRobtopSong, bool unkBool, bool isMusicLibrary, int unk) {
		if (!CustomSongWidget::init(songInfo, songDelegate, showSongSelect, showPlayMusic, showDownload, isRobtopSong, unkBool, isMusicLibrary, unk)) return false;

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
