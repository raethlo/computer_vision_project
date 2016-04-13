require 'find'
require 'pry-byebug'
require 'zip'

class Unpacker
  def unpack
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
      File.open('emotions.csv', 'w') do |csv|
        pictures.each do |p|
          picture_path = p[:path_in_zip]

          picture = zip_file.glob(picture_path).first
          if picture
            puts "Extracting #{picture.name}"
            file_dir = File.split(picture_path).first
            FileUtils.mkdir_p(file_dir) unless Dir.exists?(file_dir)
            picture.extract(picture_path) { true }
            csv.puts "#{picture_path},#{p[:emotion]}"
          else
            puts 'fuckup'
          end
        end
      end
    end
  end
end

u = Unpacker.new
u.unpack
