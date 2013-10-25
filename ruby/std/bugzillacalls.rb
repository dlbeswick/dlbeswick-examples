require 'cgi'
require 'xmlrpc/client'

#-This file contains all Bugzilla Webservice calls, transcripted for an
#easier use in Ruby.
#
#===BugzillaCalls method list :
#* new [stable]
#* parse_cookie [stable]
#* product_list [stable]
#* os [stable]
#* priorities [stable]
#* components [stable]
#* severities [stable]
#* platforms [stable]
#* bug_id (alias of get) [stable]
#* new_bug (alias of create_new_bug) [stable]
#* new_comment (alias of add_comment) [stable]
#* bugzilla_version (alias of version) [stable]
#* bugzilla_plugins (alias of plugins) [stable]
#* bugzilla_timezone (alias of timezone) [stable]
#* legal_values_for (alias for legal_values) [stable]
#* selectable_products (alias for get_selectable_products) [stable]
#* enterable_products (alias for get_enterable_products) [stable]
#* accesible_products (alias for get_accessible_products) [stable]
#* products (alias for get_products) [stable]
#* new_account (alias for create_new_account) [stable]
#
#End of documentation : 22.04.2008
#
#Class author : Tizian Schmidlin
class BugzillaCalls < XMLRPC::Client

    class << self
      attr_accessor :product_id
    end
    
    # ======================================================================== #
    # Class specific calls                                                     #
    # ======================================================================== #
        
    #===History :
    #Problem :
    #Bugzilla sends _2_ cookies when you login. The XMLRPC class puts them
    #together. BUT, the end of the cookies sent by Bugzilla have no semicolon
    #(;) so as they are merged, the cookie becomes corrupted and Bugzilla 
    #doesn't analyze it well and throws a Login Error when you try to add a
    #bug.
    #
    #Solution :
    #
    #CGI::Cookie::parse() solves the problem by parsing the cookie proprely.
    #
    #Parse proprely the cookies got by Bugzilla.  
    def parse_cookie
      c = CGI::Cookie::parse(@cookie)
      @cookie = c.keys.collect{|x| "#{x}=#{c[x].value}"}.join('; ');
    end
    
    # Get the product list for the current user
    def product_list
      a = selectable_products
      l = products a
      l
    end
    
    # Backwards compatibility aliases
    alias get_product_list product_list
    
    # Returns the os' which have been defined in Bugzilla
    #
    # +id+ ::
    #   Id of the product you want the legal value from
    def os id
      return legal_values_for 'op_sys', id
    end
    
    # Returns the proprietis which have been defined in Bugzilla
    #
    # +id+ ::
    #   Id of the product you want the legal value from
    def priorities id
      return legal_values_for 'priority', id
    end
    
    # Returns the components which have been defined in Bugzilla
    #
    # +id+ ::
    #   Id of the product you want the legal value from
    def components id
      return legal_values_for 'component', id
    end
    
    # Returns the severities which have been defined in Bugzilla
    #
    # +id+ ::
    #   Id of the product you want the legal value from
    def severities id
      return legal_values_for 'severity', id
    end
    
    # Returns the platforms which have been defined in Bugzilla
    #
    # +id+ ::
    #   Id of the product you want the legal value from
    def platforms id
      return legal_values_for 'platform', id
    end
    
    # ======================================================================== #
    # Bugzilla calls                                                           #
    # ======================================================================== #
    
    # These methods call the homonymous Bugzilla call.
    # There is a Ruby noramlized alias for each one, except the
    # Bugzilla::Webservice::User methods which are very explicit.
      
    # ************************************************************************ #
    # Bugzilla::Webservice::Bug calls                                          #
    # ************************************************************************ #
  
    # Get bugs by an array of ids
    #
    # +ids+ ::
    #   List of ids you want the bug informations from.
    def get(ids =[])
      return call('Bug.get', ids)
    end
  
#Create a bug
#
#Documentation from http://www.bugzilla.org/docs/tip/api/Bugzilla/Webservice/Bug.html
#
#==Params
#
#Some params must be set, or an error will be thrown. These params are marked Required.
#
#Some parameters can have defaults set in Bugzilla, by the administrator. If these parameters have defaults set, you can omit them. These parameters are marked Defaulted.
#
#Clients that want to be able to interact uniformly with multiple Bugzillas should always set both the params marked Required and those marked Defaulted, because some Bugzillas may not have defaults set for Defaulted parameters, and then this method will throw an error if you don't specify them.
#
#The descriptions of the parameters below are what they mean when Bugzilla is being used to track software bugs. They may have other meanings in some installations.
#
#+product+ (string) *Required* :: 
    # The name of the product the bug is being filed against.
#
#+component+ (string) *Required* :: 
    # The name of a component in the product above.
#
#+summary+(string) *Required* :: 
    # A brief description of the bug being file.
#
#+version+(string) *Required* :: 
    # A version of the product above; the version the bug was found in.
#
#+description+(string) *Defaulted* :: 
    # The initial description for this bug. Some Bugzilla installations require this to not be blank.
#
#+op_sys+(string) *Defaulted* :: 
    # The operating system the bug was discovered on.
#
#+platform+(string) *Defaulted* :: 
    # What type of hardware the bug was experienced on.
#
#+priority+(string) *Defaulted* :: 
    # What order the bug will be fixed in by the developer, compared to the developer's other bugs.
#
#+severity+(string) *Defaulted* :: 
    # How severe the bug is.
#
#+alias+(string) :: 
    # A brief alias for the bug that can be used instead of a bug number when accessing this bug. Must be unique in all of this Bugzilla.
#
#+assigned_to+(username) :: 
    # A user to assign this bug to, if you don't want it to be assigned to the component owner.
#
#+cc+(array) :: 
    # An array of usernames to CC on this bug.
#
#+qa_contact+(username) :: 
    # If this installation has QA Contacts enabled, you can set the QA Contact here if you don't want to use the component's default QA Contact.
#
#+status+(string) :: 
    # The status that this bug should start out as. Note that only certain statuses can be set on bug creation.
#
#+target_milestone+(string) :: 
    # A valid target milestone for this product.
#
# In addition to the above parameters, if your installation has any custom fields, you can set them just by passing in the name of the field and its value as a string.
    def create_new_bug(bug={})
      return call('Bug.create', bug)
    end
    
#Add a comment to a specified bug

#Documentation from http://www.bugzilla.org/docs/tip/api/Bugzilla/Webservice/Bug.html
#==Params
#+id+ (int) *Required* ::
# The id or alias of the bug to append a comment to.
#+comment+ (string) *Required* ::
# The comment to append to the bug.
#+_private+ (boolean) ::
# If set to true, the comment is private, otherwise it is assumed to be public.
#+work_time+ (double) ::
# Adds this many hours to the "Hours Worked" on the bug. If you are not in the time tracking group, this value will be ignored.

    def add_comment(id, comment="", _private=false, work_time=nil)
      comment = {'id' => id, 'comment' => comment}
      comment.store('private', _private) if _private
      comment.store('work_time', work_time) unless work_time.nil?
      return call('Bug.add_comment', comment)
    end
    
    # ************************************************************************ #
    # Bugzilla::Webservice::Bugzilla calls                                     #
    # ************************************************************************ #
    
    # Get the installed Bugzilla version
    def version
      return call('Bugzilla.version')
    end
  
    # Get a list of the installed Bugzilla plugins
    def plugins
      return call('Bugzilla.plugins')
    end
  
    # Get the installed Bugzilla timezone
    def timezone
      return call('Bugzilla.tomezone')
    end
  
    # ************************************************************************ #
    # Bugzilla::Webservice::Products calls                                     #
    # ************************************************************************ #
  
    # Returns what values are allowed for a particular field.
    #
    # +field+ ::
    #   The name of the field you want the legal values from.
    #
    # +product_id+ ::
    #   The id of the product you want the field values for.
    def legal_values(field, product_id=nil)
      raise 'No product id set' if product_id.nil?
      return call('Bug.legal_values', {'field' => field, \
                  'product_id' => product_id.to_i})
    end
    
    # Returns an array of product id which are viewable by the user.
    def get_selectable_products
      return call('Product.get_selectable_products')
    end
  
    # Returns an array of product ids for which the user can enter bugs.
    def get_enterable_products
      return call('Product.get_enterable_products')
    end
  
    # Returns an array of product ids for which the user can view and enter bugs.
    def get_accessible_products
      return call('Product.get_accessible_products')
    end
  
    # Returns an array of product infos about the product ids specified in ids.
    #
    # +ids+ ::
    #   Ids of the products you want more information from.
    def get_products(ids=[])
      ids = [ids] unless ids.is_a? Array
      return call('Product.get_products', {:ids => ids})
    end
  
    # ************************************************************************ #
    # Bugzilla::Webservice::User calls                                         #
    # ************************************************************************ #
    
    # Try to login and return the user id
    #
    # +user+ ::
    #   Specify the username to connect with to the Bugzilla application.
    #
    # +password+ ::
    #   The password for this user account.
    def login(user=nil, password=nil)
      # If the user or password have already been set and no new informations
      # are given to the function, take these old informations.
      @user = user if @user.nil? 
      @password = password if @password.nil?
      rep = call('User.login', {'login' => @user.to_s, 'password' => @password,\
                 'remember' => 1})
      # Parse the cookie stored in @cookie.
      parse_cookie
      rep
    end
  
    # Logout from actual session
    def logout
      call('User.logout')
    end
  
    # Send an e-mail to the user and offer to create an account
    #
    # +email+ ::
    #   E-mail of the person you want to offer an account to.
    def offer_account_by_email(email)
      return call('User.offer_account_by_email', {'email' => email})
    end
  
    # Create a new account
    #
    # +email+ ::
    #   E-mail address used to create this account
    #
    # +full_name+(optional) ::
    #   Full name of the person how owns the e-mail address.
    #
    # +password+(options) ::
    #   A password for the newly created account.
    def create_new_account(email, full_name=nil, password=nil)
      return call('User.create', {'email' => email, 'full_name' => full_name,\
      'password' => password})
    end

    # ************************************************************************ #
    # Alias list                                                               #
    # alias newname oldname                                                    #
    # ************************************************************************ #
    
    alias bug_id get
    alias new_bug create_new_bug
    alias new_comment add_comment
    alias bugzilla_version version
    alias bugzilla_plugins plugins
    alias bugzilla_timezone timezone
    alias legal_values_for legal_values
    alias selectable_products get_selectable_products
    alias enterable_products get_enterable_products
    alias accesible_products get_accessible_products
    alias products get_products
    alias new_account create_new_account
    
  #end

end
