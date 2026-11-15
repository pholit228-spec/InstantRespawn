#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// ---------------------------------------------------------------
// 1) Instant respawn: no delay after death
// ---------------------------------------------------------------
class $modify(InstantPlayLayer, PlayLayer) {
	struct Fields {
		// true after we reset manually; the game's own delayed reset
		// may still be queued, and we must not let it fire later
		bool m_staleResetPossible = false;
	};

	void destroyPlayer(PlayerObject* player, GameObject* object) {
		PlayLayer::destroyPlayer(player, object);

		if (!Mod::get()->getSettingValue<bool>("instant-respawn")) {
			m_fields->m_staleResetPossible = false;
			return;
		}

		// the anti-cheat spike is not a real death
		if (object == m_anticheatSpike) return;

		// make sure somebody actually died (other mods may cancel the death)
		if (!m_player1->m_isDead && !m_player2->m_isDead) return;

		// cancel the game's delayed reset (the ~1s wait) and reset right now
		this->stopActionByTag(0x10);
		m_fields->m_staleResetPossible = true;
		this->delayedResetLevel();
	}

	void delayedResetLevel() {
		// a leftover delayed reset firing while the player is alive -> ignore it
		if (m_fields->m_staleResetPossible && !m_player1->m_isDead && !m_player2->m_isDead) {
			m_fields->m_staleResetPossible = false;
			return;
		}
		PlayLayer::delayedResetLevel();
	}
};

// ---------------------------------------------------------------
// 2) Esc in the pause menu can't exit the level by accident
// ---------------------------------------------------------------
class $modify(SafePauseLayer, PauseLayer) {
	void keyBackClicked() {
		auto mode = Mod::get()->getSettingValue<std::string>("esc-mode");

		if (mode == "Do nothing") return;

		if (mode == "Resume level") {
			this->onResume(nullptr);
			return;
		}

		PauseLayer::keyBackClicked();
	}
};
