import sys

if __name__ == '__main__':
	if len(sys.argv) != 3:
		print('[usage]:python3 mapping.py <input_file> <output_file>')
		exit()

	dictionary = {'ㄅ':[], 'ㄆ':[], 'ㄇ':[], 'ㄈ':[], 'ㄉ':[], 'ㄊ':[], 'ㄋ':[], 'ㄌ':[], 'ㄍ':[], 'ㄎ':[], 'ㄏ':[], 'ㄐ':[], 'ㄑ':[], 'ㄒ':[], 'ㄓ':[], 'ㄔ':[], 'ㄕ':[], 'ㄖ':[], 'ㄗ':[], 'ㄘ':[], 'ㄙ':[], 'ㄧ':[], 'ㄨ':[], 'ㄩ':[], 'ㄚ':[], 'ㄛ':[], 'ㄜ':[], 'ㄝ':[], 'ㄞ':[], 'ㄟ':[], 'ㄠ':[], 'ㄡ':[], 'ㄢ':[], 'ㄣ':[], 'ㄤ':[], 'ㄥ':[], 'ㄦ':[]}
	char_list = []
	input_file = open(sys.argv[1], 'r', encoding='big5-hkscs')
	for line in input_file:
		#print(line, end='')
		split_sp = line.split()
		char = split_sp[0]
		zhu = split_sp[1].split('/')
		char_list.append(char)
		for z in zhu:
			initial = z[0]
			if(char not in dictionary[initial]):
				dictionary[initial].append(char)
	input_file.close()
	'''for z, lst in dictionary.items():
		print(z, ':', lst)
	print(char_list)'''
	output_file = open(sys.argv[2], 'w', encoding='big5-hkscs')
	for z, lst in dictionary.items():
		if len(lst) > 0:
			wr_str = z + '\t' + ' '.join(lst) + '\n'
			output_file.write(wr_str)
	for ch in char_list:
		output_file.write(ch + '\t' + ch + '\n')
	output_file.close()





