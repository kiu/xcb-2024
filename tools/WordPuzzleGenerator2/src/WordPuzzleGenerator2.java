import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class WordPuzzleGenerator2 {

	public enum Orientation {
		LR, RL, TB, BT
	}

	public class WordPlace {
		public int x;
		public int y;
		public Orientation o;

		public WordPlace(int x, int y, Orientation o) {
			this.x = x;
			this.y = y;
			this.o = o;
		}
	}

	public class StringLengthComparator implements Comparator<String> {
		@Override
		public int compare(String s1, String s2) {
			return Integer.compare(s2.length(), s1.length());
		}
	}

	private static final String LETTERS_ALL = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	public static final boolean DEBUG = false;
	private final boolean strategy_packed = true;
	private final boolean letters_fill_all = false;

	private final String fileWord = "wpg-words.txt";
	private final String fileMask = "wpg-mask.txt";

	private int w = 0;
	private int h = 0;
	private final char m = '*';

	private int max_achieved = 0;

	private char[][] space_org;
	private char[][] space;

	private final List<String> words = new ArrayList<String>();
	private final List<Character> letters = new ArrayList<Character>();
	private List<Orientation> orientationsEnabled = new ArrayList<Orientation>();

	private final SecureRandom rnd = new SecureRandom();

	public WordPuzzleGenerator2() {
		this.orientationsEnabled.add(Orientation.LR);
		this.orientationsEnabled.add(Orientation.TB);

		maskDetermineSize();
		maskLoad();
		wordsLoad();
	}

	private void maskLoad() {
		this.space_org = new char[this.w][this.h];

		for (int y = 0; y < this.h; y++) {
			for (int x = 0; x < this.w; x++) {
				this.space_org[x][y] = '*';
			}
		}

		BufferedReader reader;
		try {
			reader = new BufferedReader(new FileReader(this.fileMask));
			String line;
			int x = 0;
			int y = 0;
			while ((line = reader.readLine()) != null) {
				if (DEBUG) {
					System.out.println(line);
				}
				for (int cx = 0; cx < line.length(); cx += 2) {
					this.space_org[x][y] = line.charAt(cx);
					x++;
				}
				x = 0;
				y++;
			}

			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private void wordsLoad() {
		this.words.clear();
		this.letters.clear();

		BufferedReader reader;
		try {
			reader = new BufferedReader(new FileReader(this.fileWord));
			String line;
			while ((line = reader.readLine()) != null) {				
				wordListAdd(this.words, line, this.letters);				
			}

			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		Collections.sort(words, new StringLengthComparator());
	}

	private void wordListAdd(List<String> words, String word, List<Character> letters) {
		word = word.trim();
		word = word.toUpperCase();
		if (word.isBlank()) {
			return;
		}
		if (words.contains(word)) {
			return;
		}
		words.add(word);

		for (Character c : word.toCharArray()) {
			if (letters.contains(c)) {
				continue;
			}
			if (c == ' ' || c == this.m) {
				continue;
			}
			if (DEBUG) {
				System.out.println("Letter: " + c);
			}
			letters.add(c);
		}
	}

	private void maskDetermineSize() {
		int maxw = 0;
		int maxh = 0;

		BufferedReader reader;
		try {
			reader = new BufferedReader(new FileReader(this.fileMask));
			String line;
			while ((line = reader.readLine()) != null) {
				maxh++;
				int linec = line.length();
				if (linec > maxw) {
					maxw = linec;
				}
			}
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		this.w = maxw / 2 + 1;
		this.h = maxh;
		if (DEBUG) {
			System.out.println("Size: " + this.w + " x " + this.h);
		}
	}

	private int spaceCheck(String word, int x, int y, Orientation o) {
		if (Orientation.LR == o) {
			if (x + word.length() > this.w) {
				return -1;
			}
			int countOverlap = 0;
			for (int i = 0; i < word.length(); i++) {
				boolean overlap = this.space[x + i][y] == word.charAt(i);
				if (this.space[x + i][y] != this.m && !overlap) {
					return -2;
				}
				if (overlap) {
					countOverlap++;
				}
			}
			return countOverlap;
		}

		if (Orientation.RL == o) {
			if (x - word.length() < 0) {
				return -3;
			}
			int countOverlap = 0;
			for (int i = 0; i < word.length(); i++) {
				boolean overlap = this.space[x - i][y] == word.charAt(i);
				if (this.space[x - i][y] != this.m && !overlap) {
					return -4;
				}
				if (overlap) {
					countOverlap++;
				}
			}
			return countOverlap;
		}

		if (Orientation.TB == o) {
			if (y + word.length() > this.h) {
				return -5;
			}
			int countOverlap = 0;
			for (int i = 0; i < word.length(); i++) {
				boolean overlap = this.space[x][y + i] == word.charAt(i);
				if (this.space[x][y + i] != this.m && !overlap) {
					return -6;
				}
				if (overlap) {
					countOverlap++;
				}
			}
			return countOverlap;
		}

		if (Orientation.BT == o) {
			if (y - word.length() < 0) {
				return -7;
			}
			int countOverlap = 0;
			for (int i = 0; i < word.length(); i++) {
				boolean overlap = this.space[x][y - i] == word.charAt(i);
				if (this.space[x][y - i] != this.m && !overlap) {
					return -8;
				}
				if (overlap) {
					countOverlap++;
				}
			}
			return countOverlap;
		}

		return -9;

	}

	private void spacePlace(String word, int x, int y, Orientation o) {
		if (Orientation.LR == o) {
			for (int i = 0; i < word.length(); i++) {
				this.space[x + i][y] = word.charAt(i);
			}
		}
		if (Orientation.RL == o) {
			for (int i = 0; i < word.length(); i++) {
				this.space[x - i][y] = word.charAt(i);
			}
		}
		if (Orientation.TB == o) {
			for (int i = 0; i < word.length(); i++) {
				this.space[x][y + i] = word.charAt(i);
			}
		}

		if (Orientation.BT == o) {
			for (int i = 0; i < word.length(); i++) {
				this.space[x][y - i] = word.charAt(i);
			}
		}
	}

	private void saveResult() {
		BufferedWriter writer;

		try {
			writer = new BufferedWriter(new FileWriter(this.fileWord + ".created_solution.txt"));
			System.out.println("---");
			for (int y = 0; y < this.h; y++) {
				StringBuffer sb = new StringBuffer();
				boolean first = true;
				for (int x = 0; x < this.w; x++) {
					if (first) {
						first = false;

					} else {
						sb.append(" ");
					}
					sb.append(this.space[x][y]);
				}
				writer.write(sb.toString() + "\n");
				System.out.println(sb.toString());
			}
			System.out.println("---");
			writer.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		try {
			writer = new BufferedWriter(new FileWriter(this.fileWord + ".created.txt"));
			System.out.println("---");
			for (int y = 0; y < this.h; y++) {
				StringBuffer sb = new StringBuffer();
				boolean first = true;
				for (int x = 0; x < this.w; x++) {
					if (first) {
						first = false;
					} else {
						sb.append(" ");
					}
					if (this.space[x][y] == this.m) {
						if (this.letters_fill_all) {
							this.space[x][y] = LETTERS_ALL.charAt(rnd.nextInt(LETTERS_ALL.length()));
						} else {
							this.space[x][y] = this.letters.get(this.rnd.nextInt(this.letters.size()));
						}
					}
					sb.append(this.space[x][y]);
				}
				writer.write(sb.toString() + "\n");
				System.out.println(sb.toString());
			}
			System.out.println("---");
			writer.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public boolean solve() {
		this.space = new char[this.w][this.h];
		for (int y = 0; y < this.h; y++) {
			for (int x = 0; x < this.w; x++) {
				this.space[x][y] = this.space_org[x][y];
			}
		}

		int i = 0;
		for (String word : this.words) {
			i++;
			if (DEBUG) {
				System.out.println("Processing: " + word);
			}
			List<WordPlace> potentials = new ArrayList<WordPlace>();

			int scoreHighest = 0;
			for (int y = 0; y < this.h; y++) {
				for (int x = 0; x < this.w; x++) {
					for (Orientation o : this.orientationsEnabled) {
						int score = spaceCheck(word, x, y, o);
						if (score < 0) {
							continue;
						}
						if (this.strategy_packed) {
							if (score < scoreHighest) {
								continue;
							}
							if (score > scoreHighest) {
								scoreHighest = score;
								potentials.clear();
							}
						}
						potentials.add(new WordPlace(x, y, o));
					}
				}
			}

			if (potentials.isEmpty()) {
				if (i > this.max_achieved) {
					this.max_achieved = i;
				}
				System.err.println(
						"Could not place: " + word + " (" + i + "/" + this.words.size() + ") -- " + this.max_achieved);
				return false;
			}

			WordPlace pickedPlace = potentials.get(this.rnd.nextInt(potentials.size()));
			spacePlace(word, pickedPlace.x, pickedPlace.y, pickedPlace.o);
		}

		saveResult();

		return true;

	}

	public static void main(String[] args) {
		WordPuzzleGenerator2 wpg = new WordPuzzleGenerator2();

		for (int i = 0; i < 50000; i++) {
			if (wpg.solve()) {
				System.out.println("SUCCESS :)");
				return;
			}
		}
		System.out.println("FAILED :(");
	}

}
