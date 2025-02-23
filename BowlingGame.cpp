#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

constexpr uint8_t PINS {10}; // Number of pins per frame
constexpr uint8_t FRAMES {10}; // Number of frames
constexpr uint8_t BASE_SCORE {10}; // Base score for strike and spare

/**
 * @class Roll
 * @brief Stores pin count with single roll
 */
class Roll {
public:
	explicit Roll(uint8_t p) : m_pins(p) {}
	uint8_t pins() const {
		return m_pins;
	}

private:
	uint8_t m_pins;
};

/**
 * class Frame
 * @brief Base class representing a bowling frame
 */
class Frame {
public:
	Frame(uint8_t r1, uint8_t r2) : roll1(r1), roll2(r2)
	{}
	virtual ~Frame() = default;

	/**
	 * @brief Pure virtual function to compute the score for a frame.
	 */
	virtual uint16_t score() const = 0;

	/**
	 * @brief Pure virtual function to retrun a formatted frame representation
	 *        e.g. for strike 'X'
	 *             for spare '2 /'
	 *             for normal '2 5'
	 *             for 10th frame '2 / 6'
	 */
	virtual std::string frameType() const = 0;

	uint8_t firstRoll() const {
		return roll1.pins();
	}

	uint8_t secondRoll() const {
		return roll2.pins();
	}

	bool isStrike() const {
		return firstRoll() == BASE_SCORE;
	}

	bool isSpare() const {
		return firstRoll() + secondRoll() == BASE_SCORE && firstRoll() != BASE_SCORE;
	}

protected:
	Roll roll1, roll2;
};

/**
 * @ class NormalFrame
 * @brief Represents a normal bowling frame (not a spare or strike)
 */
class NormalFrame : public Frame {
public:
	NormalFrame(uint8_t r1, uint8_t r2) : Frame(r1, r2) {}
	virtual ~NormalFrame() = default;

	uint16_t score() const override {
		return roll1.pins() + roll2.pins();
	}

	std::string frameType() const override {
		return std::to_string(roll1.pins()) + " " + std::to_string(roll2.pins());
	}
};

/**
 * @class SpareFrame
 * @brief Represents a spare bowling frame
 */
class SpareFrame : public Frame {
public:
	SpareFrame(uint8_t r1) : Frame(r1, PINS - r1) {}
	virtual ~SpareFrame() = default;

	uint16_t score() const override {
		return BASE_SCORE;
	}

	std::string frameType() const override {
		return std::to_string(roll1.pins()) + " / ";
	}
};

/**
 * @ class SpareFrame
 * @brief Represents a strike bowling frame
 */
class StrikeFrame : public Frame {
public:
	StrikeFrame() : Frame(PINS, 0) {}
	virtual ~StrikeFrame() = default;

	uint16_t score() const override {
		return BASE_SCORE;
	}

	std::string frameType() const override {
		return "X";
	}
};

/**
 * @class TenthFrame
 * @brief Represents the special 10th frame, which allows a third roll if a spare or strike is rolled
 */
class TenthFrame : public Frame {
private:
	uint8_t thirdRoll;
	bool thirdRollAllowed;
public:
	using Frame::isStrike;
	TenthFrame(uint8_t r1, uint8_t r2, uint8_t r3 = 0)
		: Frame(r1, r2), thirdRoll(r3), thirdRollAllowed(false) {
		if (isStrike(r1) || isSpare()) {
			thirdRollAllowed = true;
		}
	}

	virtual ~TenthFrame() = default;

	uint16_t score() const override {
		return roll1.pins() + roll2.pins() + thirdRoll;
	}

	bool isStrike(uint8_t pins) const {
		return pins == PINS;
	}

	std::string frameType() const override {
		std::string frame = (isStrike(firstRoll()) ? " X" : std::to_string(firstRoll()));
		frame += (isStrike(secondRoll()) ? " X" : isSpare() ? " /" : " " + std::to_string(secondRoll()));
		if (thirdRollAllowed) {
			frame += (isStrike(thirdRoll) ? "X " : " ") + std::to_string(thirdRoll);
		}
		return frame;
	}
};

class FrameFactory {
public:
	static std::unique_ptr<Frame> createFrame(uint8_t frameIndex, uint8_t r1, uint8_t r2 = 0, uint8_t r3 = 0) {
		if (frameIndex == FRAMES - 1) { // 10th frame
			return std::make_unique<TenthFrame>(r1, r2, r3);
		} else if (r1 == BASE_SCORE) { // Strike
			return std::make_unique<StrikeFrame>();
		} else if (r1 + r2 == BASE_SCORE) { // Spare
			return std::make_unique<SpareFrame>(r1);
		} else {
			return std::make_unique<NormalFrame>(r1, r2);
		}
	}
};


/**
 * @class BowlingGame
 * @brief Simulates the bowling game and calculates scores
 */
class BowlingGame {
public:
	void roll(uint8_t pins) {
		m_rolls.push_back(pins);
	}

	void processFrames() {
		m_frames.clear();
		size_t i = 0;

		while (m_frames.size() < FRAMES - 1 && i < m_rolls.size()) {
			uint8_t r1 = m_rolls[i++];
			uint8_t r2 = (r1 != PINS && i < m_rolls.size()) ? m_rolls[i++] : 0;
			m_frames.push_back(FrameFactory::createFrame(m_frames.size(), r1, r2));
		}

		// 10th frame
		if (i < m_rolls.size()) {
			uint8_t r1 = m_rolls[i++];
			uint8_t r2 = (i < m_rolls.size()) ? m_rolls[i++] : 0;
			uint8_t r3 = (i < m_rolls.size() && (r1 == PINS || r1 + r2 == PINS)) ? m_rolls[i++] : 0;
			m_frames.push_back(FrameFactory::createFrame(FRAMES - 1, r1, r2, r3));
		}
	}


	void displayBoard() {
		std::cout << "\nFrame |";
		for (size_t i = 1; i <= FRAMES; i++) {
			std::cout << " " << std::setw(4) << i << " |";
		}
		std::cout << "\n-----------------------------------------------------------------------------\n";

		std::cout << "Rolls |";
		for (const auto& frame : m_frames) {
			std::cout << " " << std::setw(4) << frame->frameType() << " |";
		}
		std::cout << "\n-----------------------------------------------------------------------------\n";

		std::cout << "Score |";
		uint16_t runningScore = 0;
		for (size_t i = 0; i < m_scores.size(); i++) {
			runningScore = m_scores[i];
			std::cout << " " << std::setw(4) << runningScore << " |";
		}
		std::cout << "\n";
	}

	int calculateScore() {

		int totalScore = 0;
		size_t rollIndex = 0;
		m_scores.clear();

		for (size_t i = 0; i < m_frames.size(); i++) {
			int frameScore = m_frames[i]->score();

			if (i < FRAMES - 1) { // First 9 frames need bonus calculations
				if (m_frames[i]->isStrike()) {
					frameScore += strikeBonus(rollIndex);
				} else if (m_frames[i]->isSpare()) {
					frameScore += spareBonus(rollIndex);
				}
			}

			totalScore += frameScore;
			m_scores.push_back(totalScore);
			rollIndex += m_frames[i]->isStrike() ? 1 : 2; // Move index correctly
		}

		return totalScore;
	}


private:
	std::vector<uint8_t> m_rolls;
	std::vector<uint16_t> m_scores;
	std::vector<std::unique_ptr<Frame>> m_frames;

	uint8_t strikeBonus(int index) {
		if (index + 1 < m_rolls.size()) {
			uint8_t bonus = m_rolls[index + 1]; // First bonus roll
			if (index + 2 < m_rolls.size()) {
				bonus += m_rolls[index + 2]; // Second bonus roll
			}
			return bonus;
		}
		return 0;
	}
	uint8_t spareBonus(int index) {
		if (index + 2 < m_rolls.size()) {
			return m_rolls[index + 2]; // next roll after the spare
		}
		return 0;
	}

};

/**
 * @brief Helper function to validate user input
 */
uint8_t getValidatedInput(const std::string &prompt) {
	int pins;
	while (true) {
		std::cout << prompt;
		std::cin >> pins;

		if (std::cin.fail() || pins < 0 || pins > PINS) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cerr << "Invalid input! Enter a number between 0 and 10.\n";
		} else {
			return static_cast<uint8_t>(pins);
		}
	}
}

/**
 * @brief Process user input for the bowling game.
 */
void processUserInput(BowlingGame &game) {
	uint8_t frameCount = 0;

	while (frameCount < FRAMES) {
		std::cout << "-----> Roll for frame " << static_cast<int>(frameCount + 1) << std::endl;

		uint8_t firstRoll = getValidatedInput("Enter Roll 1: ");
		game.roll(firstRoll);

		if (frameCount == FRAMES - 1) {  // 10th frame logic
			std::string prompt = (firstRoll == PINS) ? "Enter Extra Roll: " : "Enter Roll 2: ";
			uint8_t secondRoll;
			secondRoll = getValidatedInput(prompt);
			game.roll(secondRoll);


			if (firstRoll == PINS || firstRoll + secondRoll == PINS) {  // Strike or Spare
				uint8_t thirdRoll = getValidatedInput("Enter Extra Roll: ");
				game.roll(thirdRoll);
			}
			return;
		}

		if (firstRoll == PINS) {  // Strike, move to next frame
			frameCount++;
			continue;
		}

		uint8_t secondRoll = getValidatedInput("Enter Roll 2: ");
		game.roll(secondRoll);

		frameCount++;
	}
}


int main() {

	BowlingGame game;
#ifdef USER_DRIVEN
	processUserInput(game);
#else
	std::vector<uint8_t> rolls {1, 4, 4, 5, 6, 4, 5, 5, 10, 0, 1, 7, 3, 6, 4, 10, 2, 8, 6};
	for(auto r : rolls) {
		game.roll(r);
	}
#endif
	game.processFrames();
	int totalSocre = game.calculateScore();
	game.displayBoard();

	std::cout << "Total score: " << totalSocre << std::endl;
	return 0;
}