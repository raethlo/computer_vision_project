require 'find'
require 'pry-byebug'
require 'zip'

class Unpacker
  def load(emotions_dir_path, pictures_dir_path, zip_path)
    #  There should be only one entry and the number will range from 0-7 (i.e. 0=neutral, 1=anger, 2=contempt, 3=disgust, 4=fear, 5=happy, 6=sadness, 7=surprise)
    pictures = Find.find(emotions_dir_path).select{ |e| File.file? e }.map do |path|
      emotion = File.read(path).strip.to_i

      # binding.pry
      parts = path.split('/')
      picture_filename = parts.last.split('.').first.sub('_emotion','') + '.png'

      { path_in_zip: ((['cohn-kanade-images'] + parts[1..-2]) << picture_filename).join('/'), emotion: emotion }
    end
  end

  def extract(zip_path, pictures)
    Zip::File.open(zip_path) do |zip_file|
      pictures.each do |p|
        picture_path = p[:path_in_zip]

        picture = zip_file.glob(picture_path).first
        if picture
          puts "Extracting #{picture.name}"
          file_dir = File.split(picture_path).first
          FileUtils.mkdir_p(file_dir) unless Dir.exists?(file_dir)
          picture.extract(picture_path) { true }
        else
          puts 'fuckup'
        end
      end
    end
  end

  def save_csv(pictures, csv_name)
    File.open(csv_name, 'w') do |csv|
      pictures.each do |p|
        picture_path = p[:path_in_zip]
        if File.exists? picture_path
          csv.puts "#{File.absolute_path(picture_path)},#{p[:emotion]}"  if p[:emotion] == 5  || p[:emotion] == 6
        else
          puts 'fuckup'
        end
      end
    end
  end

  def unpack_old
    emotions_dir_path = "Emotion"
    pictures_dir_path = "Pictures"
    zip_path = "/home/raethlo/Downloads/extended-cohn-kanade-images.zip"

    #  There should be only one entry and the number will range from 0-7 (i.e. 0=neutral, 1=anger, 2=contempt, 3=disgust, 4=fear, 5=happy, 6=sadness, 7=surprise)
    pictures = Find.find(emotions_dir_path).select{ |e| File.file? e }.map do |path|
      emotion = File.read(path).strip.to_i

      # binding.pry
      parts = path.split('/')
      picture_filename = parts.last.split('.').first.sub('_emotion','') + '.png'

      {path_in_zip: ((['cohn-kanade-images'] + parts[1..-2]) << picture_filename).join('/'), emotion: emotion}
    end

    Zip::File.open(zip_path) do |zip_file|
      binding.pry
      File.open('emotions_happysad', 'w') do |csv|
        pictures.each do |p|
          picture_path = p[:path_in_zip]

          picture = zip_file.glob(picture_path).first
          if picture
            puts "Extracting #{picture.name}"
            file_dir = File.split(picture_path).first
            FileUtils.mkdir_p(file_dir) unless Dir.exists?(file_dir)
            picture.extract(picture_path) { true }
            csv.puts "#{File.absolute_path(picture_path)},#{p[:emotion]}" if p[:emotion] == 5 || p[:emotion] == 6
          else
            puts 'fuckup'
          end
        end
      end
    end
  end
end

emotions_dir_path = "Emotion"
pictures_dir_path = "Pictures"
zip_path = "/home/raethlo/Downloads/extended-cohn-kanade-images.zip"
u = Unpacker.new
pictures = u.load(emotions_dir_path, pictures_dir_path, zip_path)
# u.extract(pictures);
u.save_csv(pictures, 'test.csv')

# u.unpack_old
